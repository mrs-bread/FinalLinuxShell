#define FUSE_USE_VERSION 31

#include <cstring>
#include <fuse.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

using namespace std;

vector<string> cron_commands;

string exec(const char *cmd) {
    char buffer[128];
    string result = "";
    FILE *pipe = popen(cmd, "r");
    if (!pipe)
        return "popen() failed!";
    while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    return result;
}

vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void create_cron_vfs() {
    string crontab_info = exec("crontab -l");
    cron_commands = split(crontab_info, '\n');
}

static int cron_getattr(const char *path, struct stat *stbuf) {
    memset(stbuf, 0, sizeof(struct stat));
    if (strcmp(path, "/") == 0) {
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    } else {
        for (const auto &command : cron_commands) {
            if (command.length() != 0) {
                string file_name = command.substr(10);
                if (strcmp(path + 1, file_name.c_str()) == 0) {
                    stbuf->st_mode = S_IFREG | 0444;
                    stbuf->st_nlink = 1;
                    stbuf->st_size = command.length();
                    return 0;
                }
            }
        }
        return -ENOENT;
    }
    return 0;
}

static int cron_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info *fi) {
    filler(buf, ".", nullptr, 0);
    filler(buf, "..", nullptr, 0);
    for (const auto &command : cron_commands) {
        if (command.length() != 0) {
            string file_name = command.substr(10);
            filler(buf, file_name.c_str(), nullptr, 0);
        }
    }
    return 0;
}

static int cron_open(const char *path, struct fuse_file_info *fi) {
    for (const auto &command : cron_commands) {
        if (command.length() != 0) {
            string file_name = command.substr(10);
            if (strcmp(path + 1, file_name.c_str()) == 0) {
                return 0;
            }
        }
    }
    return -ENOENT;
}

static int cron_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *fi) {
    for (const auto &command : cron_commands) {
        if (command.length() != 0) {
            string file_name = command.substr(10);
            if (strcmp(path + 1, file_name.c_str()) == 0) {
                size_t len = command.length();
                if (offset < len) {
                    if (offset + size > len)
                        size = len - offset;
                    memcpy(buf, command.c_str() + offset, size);
                } else {
                    size = 0;
                }
                return size;
            }
        }
    }
    return -ENOENT;
}

static struct fuse_operations cron_operations = {
    .getattr = cron_getattr,
    .open = cron_open,
    .read = cron_read,
    .readdir = cron_readdir,
};

int main(int argc, char *argv[]) {
    create_cron_vfs();
    return fuse_main(argc, argv, &cron_operations, nullptr);
}
