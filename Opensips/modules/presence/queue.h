//
// Created by suryaveer on 2016-10-04.
//


#include <nats.h>
#include <stdio.h>
#include <string.h>

#define SUBJ_PUBLISH "PUBLISH"
#define SUBJ_SUBSCRIBE "SUBSCRIBE"

int queue_connect(const char **serverUrls);
int publish_msg(const char* txt);
void queue_disconnect();
int setQueue( const char* subject);
int subscribeQueue(const char* subject, const char* queueGroup);


