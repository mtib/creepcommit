#include "getter.hpp"

#define LIMIT

using namespace std;

FILE *out;

void download(char *url, char *file) {
    snprintf(file, 128, "/tmp/getter_%lx.txt", time(NULL));
    char cmd[1024];
    snprintf(cmd, 1020, "rm -f \"%s\"; wget --quiet \"%s\" -O \"%s\";", file, url, file);
    system(cmd);
}

void readfile(char *file, char *mem) {
    FILE* f = fopen(file, "re");
    fread(mem, sizeof(char), 5*1024*1024, f);
    fclose(f);
}

void recordMsg(const char *msg) {
    printf("msg: %s\n", msg);
    /*
    fputs(msg, out);
    fputs("\n", out);
    */
}

void handleRepo(const char *repo_full) {
    printf("repo: %s\n", repo_full);
    
    char link[300];
    char file[130];
    snprintf(link, 299, "https://api.github.com/repos/%s/commits", repo_full);
    download(link, file);
    printf("%s -> %s\n", link, file);

/*
    char content[30 * 1024 * 1024];
    readfile(file, content);
    auto jw = nlohmann::json::parse(content);


    int length = jw.size();
    for (int i = 0; i < length; i++) {
        string msg = jw.at(i)["commit"]["message"].get<string>();
        recordMsg(msg.c_str());
    }
    */
}

void handleUser(const char *user) {
    printf("user: %s\n", user);

    char repourl[300];
    char file[130];

    snprintf(repourl, 299, "https://api.github.com/users/%s/repos", user);
    download(repourl, file);

    char content[5 * 1024 * 1024];
    readfile(file, content);
    auto jw = nlohmann::json::parse(content);

    int length = jw.size();
    for (int i = 0; i < length; i++) {
        handleRepo(jw.at(i)["full_name"].get<string>().c_str());
#ifdef LIMIT
        if(i == 2) {
            break;
        }
#endif
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("usage: %s <file> <user> [<user>...]\n", argv[0]);
        return 1;
    }
    out = fopen(argv[1], "a");
    for (int i = 2; i < argc; i++) {
        handleUser(argv[i]);
#ifdef LIMIT
        break;
#endif
    }
    fclose(out);
}
