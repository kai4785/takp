#ifndef TAIL_H
#define TAIL_H

typedef void (*tailfn)(const char* line);

void tail(const char* filename, tailfn callback);

#endif // TAIL_H
