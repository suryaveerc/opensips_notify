//
// Created by suryaveer on 2016-10-06.
//

#ifndef OPENSIPS_PUBLISH_QUEUE_MSG_HANDLER_H
#define OPENSIPS_PUBLISH_QUEUE_MSG_HANDLER_H
#include "../../str.h"
#include "../../dprint.h"
#include "subscribe.h"
#include "cJSON.h"
void handlePublishMsg(const char *msg);
void handleSubscribeMsg(const char *msg);
void sendPublishNotify(const char **key, char *pres_uri, char *etag);
//void sendSubscribeNotify(const char *key, str *pres_uri, str *subs_uri);
void sendSubscribeNotify(const char **msg);
void parseSubscription(subs_t *subs, cJSON **root);
void parseKey(const char *key, str *callid, str *to_tag, str *from_tag);




static void splitUserDomain(str *uri, str * user, str *domain)
{
    //LM_DBG(" URI -----------------------%s \n", uri->s);

    int count=0;
    if(uri->s[3] == ':' ){ uri->s = uri->s+4;}
    //LM_DBG(" URI -----------------------%s \n", uri->s);
    while(uri->s[count] != '@')
    {
        user->s[count] = uri->s[count++];
    }
    user->s[count] = '\0';

    //strcpy(user->s,temp);
    user->len = count;

    LM_DBG("User is %s %d\n", user->s, user->len);

    domain->s = uri->s+count+1;
    domain->len = uri->len-(count+5);
    LM_DBG("Domain is %s %d\n", domain->s, domain->len);

}
#endif //OPENSIPS_PUBLISH_QUEUE_MSG_HANDLER_H
