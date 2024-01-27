#pragma once

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <chrono>

#include "../winux.hpp"

#define LL long long

const int BUFFER_SIZE = 1 << 8;

inline int exec_r(const char *cmd, char *result) {
    FILE *p = r_popen(cmd);
    if (!p) return -1;

    int t = 0;
    const int result_size = sizeof(result);
    while (!feof(p)) {
        if (t + BUFFER_SIZE < result_size) {
            t += fread(result + t, 1, BUFFER_SIZE, p);
        } else {
            t += fread(result + t, 1, result_size - t - 1, p);
            break;
        }
    }

    pipe_pclose(p);
    result[t] = 0;
    return t;
}

class Cmd {
private:
#if defined(__WINDOWS_) || defined(_WIN32) || defined(_WIN64)
    PROCESS_INFORMATION p;
#else
    char p[BUFFER_SIZE];
    char result[BUFFER_SIZE];
#endif
    LL clk;  // microseconds

    inline LL micros(double s) {
        return s * 1000000.0 + 0.5;
    }

    inline LL now(std::chrono::steady_clock::time_point t) {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::steady_clock::now() - t)
            .count();
    }

public:
    char cmd[BUFFER_SIZE], name[BUFFER_SIZE] = "";

    Cmd(std::string _cmd) { memcpy(cmd, _cmd.c_str(), _cmd.length()); }

    virtual inline int start() {
        // printf("%s\n", cmd);
#if defined(__WINDOWS_) || defined(_WIN32) || defined(_WIN64)
        LPWSTR w;
        WCHAR _wt[265];
        mbstowcs(_wt, cmd, cmd.length() + 1);
        w = _wt;
        STARTUPINFOW s;
        ZeroMemory(&s, sizeof(s));
        s.cb = sizeof(s);
        ZeroMemory(&p, sizeof(p));
        if (!CreateProcessW(NULL, w, NULL, NULL, 0, 0, NULL, NULL, &s, &p)) {
            return GetLastError();
        }
#else
        pid_t pid = fork();
        if (!pid) {
            system(cmd);
            exit(0);
        }
        sprintf(p, "pgrep -P %d", pid);
#endif
        return 0;
    }

    virtual inline int run() {
        return system(cmd);
    }

    virtual inline int is_alive() {
#if defined(__WINDOWS_) || defined(_WIN32) || defined(_WIN64)
        DWORD lpExitCode = 0;
        int code = GetExitCodeProcess(p.hProcess, &lpExitCode);
        return lpExitCode == STILL_ACTIVE;
#else
        return exec_r(p, result);
#endif
    }

    virtual inline int terminate() {
        return kill();
    }

    virtual inline int kill() {
#if defined(__WINDOWS_) || defined(_WIN32) || defined(_WIN64)
        TerminateProcess(p.hProcess, 0);
        CloseHandle(p.hProcess);
        CloseHandle(p.hThread);
#else
        int t(exec_r(p, result));
        if (result[t - 1] == 10) {
            result[t - 1] = 0;
        }
        char ccc[BUFFER_SIZE];
        sprintf(ccc, "kill %s >/dev/null 2&>1", result);
        system(ccc);
#endif
        return 0;
    }

    virtual inline void join() {
#if defined(__WINDOWS_) || defined(_WIN32) || defined(_WIN64)
        WaitForSingleObject(p.hProcess, INFINITE);
#else
        while (is_alive())
            ;
#endif
    }

    virtual inline double wait(double s = -1.) {
        std::chrono::steady_clock::time_point t = std::chrono::steady_clock::now();
        if (s < 0.) {
            join();
        } else {
            LL mms = micros(s);
            while (now(t) < mms && is_alive())
                ;
        }
        terminate();
        return now(t) / 1000000.0;
    }
};

class FFplayPlayer : public Cmd {
public:
    FFplayPlayer(std::string audio) : Cmd(audio) {
        sprintf(name, "ffplay");
        sprintf(cmd, "ffplay -v quiet -nodisp -autoexit -hide_banner \"%s\"", audio.c_str());
    }
};

class MpvPlayer : public Cmd {
public:
    MpvPlayer(std::string audio) : Cmd(audio) {
        sprintf(name, "mpv");
        sprintf(cmd, "mpv --no-video --no-terminal \"%s\"", audio.c_str());
    }
};

class Mpg123Player : public Cmd {
public:
    Mpg123Player(std::string audio) : Cmd(audio) {
        sprintf(name, "mpg123");
        sprintf(cmd, "mpg123 -q \"%s\"", audio.c_str());
    }
};

class CmusPlayer : public Cmd {
public:
    CmusPlayer(std::string audio) : Cmd(audio) {
        sprintf(name, "cmus");
        sprintf(cmd, "cmus-remote -f \"%s\" >/dev/null 2&>1", audio.c_str());
    }

    inline int start() {
        return run();
    }

    inline int is_alive() {
        return 1;
    }

    inline int kill() {
        system("cmus-remote -s");
        return 0;
    }

    inline void join() {}
};
