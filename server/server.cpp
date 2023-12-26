/*
 * Copyright (C) 2013 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "UbuntuMtpDatabase.h"

#include <MtpServer.h>
#include <MtpStorage.h>

#include <iostream>

#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <libintl.h>
#include <locale.h>

using namespace android;

namespace
{
struct FileSystemConfig
{
    static const int file_perm = 0664;
    static const int directory_perm = 0755;
};

}

class MtpDaemon
{

private:
    struct passwd *userdata;

    // Mtp stuff
    MtpServer* server;
    MtpDatabase* mtp_database;

    // inotify stuff
    boost::thread notifier_thread;
    boost::thread io_service_thread;

    asio::io_service io_svc;
    asio::io_service::work work;
    asio::posix::stream_descriptor stream_desc;
    asio::streambuf buf;

    int inotify_fd;

    int watch_fd;
    int media_fd;
    
    bool enable_media_mounts;

    // storage
    std::map<std::string, MtpStorage*> removables;

    void add_storage(const char *path, const char *name, bool removable)
    {
        static int storageID = 1;

        MtpStorage *storage = new MtpStorage(
            storageID, 
            path,
            name,
            1024 * 1024 * 100,  /* 100 MB reserved space, to avoid filling the disk */
            removable,
            (uint64_t) 1024 * 1024 * 1024 * 4  /* 4GB arbitrary max file size */);

        storageID++;

        mtp_database->addStoragePath(path,
                                        name,
                                        storage->getStorageID(),
                                        true);
        server->addStorage(storage);

        if (removable) {
            removables.insert(std::pair<std::string, MtpStorage*> (name, storage));
        }
    }

    void add_mountpoint_watch(const std::string& path)
    {
        VLOG(1) << "Adding notify watch for " << path;
        watch_fd = inotify_add_watch(inotify_fd,
                                     path.c_str(),
                                     IN_CREATE | IN_DELETE);
    }

    void read_more_notify()
    {
        VLOG(1) << __PRETTY_FUNCTION__;

        stream_desc.async_read_some(buf.prepare(buf.max_size()),
                                    boost::bind(&MtpDaemon::inotify_handler,
                                                this,
                                                asio::placeholders::error,
                                                asio::placeholders::bytes_transferred));
    }

    void inotify_handler(const boost::system::error_code&,
                         std::size_t transferred)
    {
        if (!enable_media_mounts) {
            read_more_notify();
            return;
        }
        
        size_t processed = 0;
 
        while(transferred - processed >= sizeof(inotify_event))
        {
            const char* cdata = processed + asio::buffer_cast<const char*>(buf.data());
            const inotify_event* ievent = reinterpret_cast<const inotify_event*>(cdata);
            path storage_path ("/media");
     
            processed += sizeof(inotify_event) + ievent->len;
     
            storage_path /= userdata->pw_name;

            if (ievent->len > 0 && ievent->mask & IN_CREATE)
            {
                if (ievent->wd == media_fd) {
                    VLOG(1) << "media root was created for user " << ievent->name;
                    add_mountpoint_watch(storage_path.string());
                } else {
                    VLOG(1) << "Storage was added: " << ievent->name;
                    storage_path /= ievent->name;
                    add_storage(storage_path.string().c_str(), ievent->name, true);
                }
            }
            else if (ievent->len > 0 && ievent->mask & IN_DELETE)
            {
                VLOG(1) << "Storage was removed: " << ievent->name;

                // Try to match to which storage was removed.
                BOOST_FOREACH(std::string name, removables | boost::adaptors::map_keys) {
                    if (name == ievent->name) {
                        MtpStorage *storage = removables.at(name);

                        VLOG(2) << "removing storage id "
                                << storage->getStorageID();

                        server->removeStorage(storage);
                        mtp_database->removeStorage(storage->getStorageID());
                    }
                }
            }
        }

        read_more_notify();
    }

public:

    MtpDaemon(int fd):
        stream_desc(io_svc),
        work(io_svc),
        buf(1024)
    {
        userdata = getpwuid (getuid());
        
        const char* env_s = std::getenv("MTP_MEDIA_MOUNTS");
        enable_media_mounts = env_s && !strncmp(env_s, "0", 1);

        // Removable storage hacks
        inotify_fd = inotify_init();
        if (inotify_fd <= 0)
            PLOG(FATAL) << "Unable to initialize inotify";
        VLOG(1) << "using inotify fd " << inotify_fd << " for daemon";

        stream_desc.assign(inotify_fd);
        notifier_thread = boost::thread(&MtpDaemon::read_more_notify, this);
        io_service_thread = boost::thread(boost::bind(&asio::io_service::run, &io_svc));


        // MTP database.
        mtp_database = new UbuntuMtpDatabase();


        // MTP server
        server = new MtpServer(
                fd, 
                mtp_database,
                false, 
                userdata->pw_gid, 
                FileSystemConfig::file_perm, 
                FileSystemConfig::directory_perm);
    }

    void initStorage()
    {
        const char* env_entry_len = std::getenv("MTP_ENTRY_LEN");
        int mtp_len = 0;
        if (env_entry_len) {
            try {
                mtp_len = std::stoi(env_entry_len);
            } catch (std::exception& e) {
                LOG(ERROR) << "Unable to parse MTP_ENTRY_LEN:" << e.what();
                mtp_len = 0;
            }
        }
        if (0 < mtp_len) {
            for (int i = 1; i <= mtp_len; i++) {
                std::string entry_base = std::string("MTP_ENTRY_") + std::to_string(i);
                const char* env_entry_path = std::getenv((entry_base + "_PATH").c_str());
                if (!env_entry_path || 0 == strlen(env_entry_path)) {
                    LOG(ERROR) << "Unable to parse " << entry_base << "_PATH";
                    continue;
                }
                const char* env_entry_name = std::getenv((entry_base + "_NAME").c_str());
                std::string entry_name = std::to_string(i);
                if (env_entry_name && 0 < strlen(env_entry_name)) {
                    entry_name = env_entry_name;
                } else if (env_entry_name) {
                    LOG(WARNING) << "Unable to parse " << entry_base << "_NAME";
                }
                const char* env_entry_removable = std::getenv((entry_base + "_REMOVABLE").c_str());
                bool removable = env_entry_removable && !strncmp(env_entry_removable, "0", 1);
                add_storage(env_entry_path, entry_name.c_str(), removable);
            }
        }
        
        // Get any already-mounted removable storage.
        if (enable_media_mounts) {
            path p(std::string("/media/") + userdata->pw_name);
            if (exists(p)) {
                std::vector<path> v;
                copy(directory_iterator(p), directory_iterator(), std::back_inserter(v));
                for (std::vector<path>::const_iterator it(v.begin()), it_end(v.end()); it != it_end; ++it)
                {
                    add_storage(it->string().c_str(), it->leaf().c_str(), true);
                }

                // make sure we can catch any new removable storage that gets added.
                add_mountpoint_watch(p.string());
            } else {
                media_fd = inotify_add_watch(inotify_fd,
                                            "/media",
                                            IN_CREATE | IN_DELETE);
            }
        }
    }

    ~MtpDaemon()
    {
        // Cleanup
        inotify_rm_watch(inotify_fd, watch_fd);
        io_svc.stop();
        notifier_thread.detach();
        io_service_thread.join();
        close(inotify_fd);
    }

    void run()
    {
        // start the MtpServer main loop
        server->run();
    }
};

int main(int argc, char** argv)
{
    bindtextdomain("mtp-server", "/usr/share/locale");
    setlocale(LC_ALL, "");
    textdomain("mtp-server");

    LOG(INFO) << "MTP server starting...";

    int fd = open("/dev/mtp_usb", O_RDWR);
    if (fd < 0)
    {
        LOG(ERROR) << "Error opening /dev/mtp_usb, aborting now...";
        return 1;
    }
 
    try {
        MtpDaemon *d = new MtpDaemon(fd);

        d->initStorage();
        d->run();

        delete d;
    }
    catch (std::exception& e) {
        /* If the daemon fails to initialize, ignore the error but
         * make sure to propagate the message and return with an
         * error return code.
         */
        LOG(ERROR) << "Could not start the MTP server:" << e.what();
    }
}
