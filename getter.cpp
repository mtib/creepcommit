#include "getter.hpp"

//#define LIMIT

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
    if(mem[0] != '{' && mem[0] != '[') {
        mem[0] = '{';
        mem[1] = '}';
        mem[2] = '\0';
        printf("%s\n", "... no response");
    }
    fclose(f);
}

void recordMsg(const char *msg) {
    //fputs("[BEGIN]", out);
    fputs(msg, out);
    fputs("\n", out);
    //fputs("[END]", out);
}

void simplify(char* dst, const char* src) {
    bool space_mode = false;
    bool isfirst = true;
    size_t index = 0;
    size_t walk = 0;
    while (src[walk] != '\0') {
        switch (src[walk]) {
            case '\r':
            case '\n':
                if(isfirst) {
                    isfirst = false;
                    dst[index] = '.';
                    index++;
                }
            case '\t':
            case ' ':
                if (!space_mode) {
                    dst[index] = ' ';
                    index++;
                    space_mode = true;
                }
                break;
            default:
                dst[index] = src[walk];
                index++;
                space_mode = false;
                break;
        }
        walk++;
    }
}

void handleRepo(const char *repo_full) {
    printf("repo: %s\n", repo_full);
    
    char link[300];
    char file[130];
    snprintf(link, 299, "https://api.github.com/repos/%s/commits?access_token=%s", repo_full, getenv("GITHUB_AUTH"));
    download(link, file);
    //printf("%s -> %s\n", link, file);

    char *content = (char*) calloc(5 * 1024 * 1024, sizeof(char));
    readfile(file, content);
    auto jw = nlohmann::json::parse(content);
    free(content);

    int length = jw.size();
    for (int i = 0; i < length; i++) {
        string msg = jw.at(i)["commit"]["message"].get<string>();
        char *simpl = (char*) calloc(msg.size() + 10, sizeof(char));
        simplify(simpl, msg.c_str());
        recordMsg(simpl);
        free(simpl);
    }

}

void handleUser(const char *user) {
    printf("user: %s\n", user);

    char repourl[300];
    char file[130];

    snprintf(repourl, 299, "https://api.github.com/users/%s/repos?access_token=%s", user, getenv("GITHUB_AUTH"));
    download(repourl, file);
    //printf("%s -> %s\n", repourl, file);

    char *content = (char*) calloc(5 * 1024 * 1024, sizeof(char));
    readfile(file, content);

    auto jw = nlohmann::json::parse(content);
    free(content);

    int length = jw.size();
    for (int i = 0; i < length; i++) {
        handleRepo(jw.at(i)["full_name"].get<string>().c_str());
#ifdef LIMIT
        if(i == 1) {
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
    printf("using env GITHUB_AUTH as authentification\n");
    out = fopen(argv[1], "a");
    for (int i = 2; i < argc; i++) {
        handleUser(argv[i]);
#ifdef LIMIT
        break;
#endif
    }
    fclose(out);
}
