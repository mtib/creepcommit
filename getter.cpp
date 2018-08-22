/*
 * Developed by Markus Becker
 *   mtib.becker@gmail.com
 */

#include "getter.hpp"

//#define LIMIT

using namespace std;

FILE *out;
CURL *curl;
string downloadBuffer;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t s = size * nmemb;
    downloadBuffer.append((const char *) contents, s);
    return s;
}

// TODO replace download/read with in-memory solution
void download(char *url, char *file) {
    snprintf(file, CREEP_DESC_LEN, "/tmp/getter_%lx.txt", time(NULL));
    char cmd[4*CREEP_DESC_LEN];
    snprintf(cmd, 4*CREEP_DESC_LEN, "rm -f \"%s\"; wget --quiet \"%s\" -O \"%s\";", file, url, file);
    system(cmd);
}

void readfile(char *file, char *mem) {
    FILE* f = fopen(file, "re");
    fread(mem, sizeof(char), CREEP_RESPONSE_LEN, f);
    if(mem[0] != '{' && mem[0] != '[') {
        mem[0] = '{';
        mem[1] = '}';
        mem[2] = '\0';
        printf("%s\n", "... no response");
    }
    fclose(f);
}

void mem_download(char *url, char *dst) {
    downloadBuffer.clear();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_perform(curl);
    strncpy(dst, downloadBuffer.c_str(), CREEP_RESPONSE_LEN);
}

void recordMsg(const char *msg) {
    fputs(msg, out);
    fputs("\n", out);
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
    
    char link[CREEP_DESC_LEN];
    snprintf(link, CREEP_DESC_LEN, "https://api.github.com/repos/%s/commits?access_token=%s", repo_full, getenv("GITHUB_AUTH"));

    char *content = (char*) calloc(CREEP_RESPONSE_LEN, sizeof(char));
    mem_download(link, content);
    auto jw = nlohmann::json::parse(content);
    free(content);

    int length = jw.size();
    for (int i = 0; i < length; i++) {
        string msg = jw.at(i)["commit"]["message"].get<string>();
        char *simpl = (char*) calloc(msg.size(), sizeof(char));
        simplify(simpl, msg.c_str());
        recordMsg(simpl);
        free(simpl);
    }

}

void handleUser(const char *user) {
    printf("user: %s\n", user);

    char repourl[CREEP_DESC_LEN];
    snprintf(repourl, CREEP_DESC_LEN, "https://api.github.com/users/%s/repos?access_token=%s", user, getenv("GITHUB_AUTH"));
    //printf("%s -> %s\n", repourl, file);

    char *content = (char*) calloc(CREEP_RESPONSE_LEN, sizeof(char));
    mem_download(repourl, content);

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
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    for (int i = 2; i < argc; i++) {
        handleUser(argv[i]);
#ifdef LIMIT
        break;
#endif
    }
    fclose(out);
}
