// pwd.h -- there is one user. gcc's build has glibc's struct, and c4bsd.h gives
// both builds the same answer.
#ifndef C4BSD_PWD_H
#define C4BSD_PWD_H 1
struct passwd {
    char *pw_name;
    char *pw_passwd;
    int   pw_uid;
    int   pw_gid;
    char *pw_gecos;
    char *pw_dir;
    char *pw_shell;
};
#endif
