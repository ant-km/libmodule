#pragma once

#include <assert.h>
#include <hashmap.h>
#include <sys/epoll.h>
#include <stdlib.h>

#ifndef NDEBUG
#define MODULE_DEBUG printf("Libmodule: "); printf
#define MOD_ASSERT(cond, msg, ret) assert(cond)
#else
#define MODULE_DEBUG (void)
#define MOD_ASSERT(cond, msg, ret) if(!cond) { fprintf(stderr, "%s\n", msg); return ret; }
#endif


#define GET_CTX(name) \
m_context *c = NULL; \
hashmap_get(ctx, (char *)name, (void **)&c); \
MOD_ASSERT(c, "Context not found.", MOD_NO_CTX);

#define CTX_GET_MOD(name, ctx) \
module *mod = NULL; \
hashmap_get(ctx->modules, (char *)name, (void **)&mod); \
MOD_ASSERT(mod, "Module not found.", MOD_NO_MOD);

#define GET_MOD(self) \
MOD_ASSERT(self, "NULL self handler.", MOD_NO_SELF); \
GET_CTX(self->ctx) \
CTX_GET_MOD(self->name, c)

#define GET_MOD_IN_STATE(self, state) \
GET_MOD(self); \
if (!module_is(self, state)) { return MOD_WRONG_STATE; }

#define CHILDREN_LOOP(f) \
child_t *tmp = m->children; \
while (tmp) { \
    GET_MOD(tmp->self); \
    f; \
    tmp = tmp->next; \
}

/* Struct that holds self module informations, static to each module */
struct _self {
    const char *name;                     // module's name
    const char *ctx;                      // module's ctx 
};

typedef struct child {
    const self_t *self;                   // module's name
    struct child *next;                   // module's ctx 
} child_t;

/* Struct that holds data for each module */
typedef struct {
    userhook *hook;                       // module's user defined callbacks
    const void *userdata;                 // module's user defined data
    enum module_states state;             // module's state
    self_t self;                          // module's info available to external world
    int fd;                               // file descriptor to be polled
    map_t subscriptions;                  // module's subscriptions
    struct epoll_event ev;                // module's epoll event struct
    child_t *children;                    // list of children modules
} module;

typedef struct {
    int quit;
    int epollfd;
    log_cb logger;
    map_t modules;
} m_context;

int evaluate_module(void *data, void *m);

map_t ctx;
