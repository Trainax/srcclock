#ifndef CLOG_H
#define CLOG_H

#include <iostream>
#include <fstream>
#include <string>

class Clog {
std::ofstream fout;
int mode;
bool errorStream;

public:
Clog();
Clog(bool ErrorMode);
Clog(const char* filename, bool ErrorMode = false);


void setErrorStream(bool ErrorMode) {
        errorStream = ErrorMode;
}

bool streamOnFile(const char* filename);
void streamOnSTDOUT();
void closeFile();

bool good() const;

Clog& flush();

Clog& operator<<(const std::string& s);
Clog& operator<<(const char* s);
Clog& operator<<(char s);
Clog& operator<<(int s);
Clog& operator<<(double s);
Clog& operator<<(long int s);
Clog& operator<<(long long int s);

virtual ~Clog();
};

#endif // CLOG_H
