//
// Created by suryaveer on 2016-10-06.
//
#include "queue_msg_handler.h"
#include "../../dprint.h"
#include "presentity.h"
#include "RedisDBUtils.h"
#include "notify.h"
#include "../../socket_info.h"
int avail_pub=0;
pres_ev_t event = {.name.s = "presence", .name.len = 8, .content_type.s = "application/pidf+xml", .content_type.len = strlen(
        "application/pidf+xml")};

void handlePublishMsg(const char *msg)
{
    // msg == sip:a@y.com:bodyXML
    //LM_DBG("Received msg: %s\n", msg);

    int count = 4; // skip sip:
    while (msg[count] != ':') {
        count++;
    }
    char pres_uri[count + 1];
    strncpy(&pres_uri, msg, count);
    pres_uri[count++] = '\0'; // count++ to move skip :
    // remaining msg is th bodyXML
    msg = msg + count;

    sendPublishNotify(&msg, &pres_uri, NULL);

}

void handleSubscribeMsg(const char *msg)
{
    //format:  0:msg or 1:msg. 0/1 indicate availability of PUBLIHSER.
    if(msg[0] == '1')
        avail_pub = 1;
    else
        avail_pub = 0;
    msg = msg+2; //skip first 2 characters.
    sendSubscribeNotify(&msg);
}

void parseSubscription(subs_t *subs, cJSON **root)
{

    str sockinfo_str;
    int port, proto;
    str host;
    sockinfo_str.s = cJSON_GetObjectItem(*root, "socket_info")->valuestring;
    if (sockinfo_str.s) {
        sockinfo_str.len = strlen(sockinfo_str.s);

        if (parse_phostport(sockinfo_str.s, sockinfo_str.len, &host.s, &host.len, &port, &proto) < 0) {
            LM_ERR("bad format for stored sockinfo string\n");
            return -1;
        }
        subs->sockinfo = grep_sock_info(&host, (unsigned short) port, (unsigned short) proto);
    }

    subs->pres_uri.s = cJSON_GetObjectItem(*root, "presentity_uri")->valuestring;
    subs->callid.s = cJSON_GetObjectItem(*root, "callid")->valuestring;
    subs->to_tag.s = cJSON_GetObjectItem(*root, "to_tag")->valuestring;
    subs->from_tag.s = cJSON_GetObjectItem(*root, "from_tag")->valuestring;
    subs->to_user.s = cJSON_GetObjectItem(*root, "to_user")->valuestring;
    subs->to_domain.s = cJSON_GetObjectItem(*root, "to_domain")->valuestring;
    subs->from_user.s = cJSON_GetObjectItem(*root, "watcher_username")->valuestring;
    subs->from_domain.s = cJSON_GetObjectItem(*root, "watcher_domain")->valuestring;
    subs->event_id.s = cJSON_GetObjectItem(*root, "event_id")->valuestring;
    subs->local_cseq = cJSON_GetObjectItem(*root, "local_cseq")->valueint;
    subs->remote_cseq = cJSON_GetObjectItem(*root, "remote_cseq")->valueint;
    subs->expires = cJSON_GetObjectItem(*root, "expires")->valueint;
    subs->status = cJSON_GetObjectItem(*root, "status")->valueint;
    subs->reason.s = cJSON_GetObjectItem(*root, "reason")->valuestring;
    subs->record_route.s = cJSON_GetObjectItem(*root, "record_route")->valuestring;
    subs->contact.s = cJSON_GetObjectItem(*root, "contact")->valuestring;
    subs->local_contact.s = cJSON_GetObjectItem(*root, "local_contact")->valuestring;
    subs->version = cJSON_GetObjectItem(*root, "version")->valueint;
    subs->pres_uri.len = strlen(subs->pres_uri.s);
    subs->callid.len = strlen(subs->callid.s);
    subs->to_tag.len = strlen(subs->to_tag.s);
    subs->from_tag.len = strlen(subs->from_tag.s);
    subs->to_user.len = strlen(subs->to_user.s);
    subs->to_domain.len = strlen(subs->to_domain.s);
    subs->from_user.len = strlen(subs->from_user.s);
    subs->from_domain.len = strlen(subs->from_domain.s);
    subs->event_id.len = strlen(subs->event_id.s);
    subs->reason.len = strlen(subs->reason.s);
    subs->record_route.len = strlen(subs->record_route.s);
    subs->contact.len = strlen(subs->contact.s);
    subs->local_contact.len = strlen(subs->local_contact.s);
    LM_DBG("callid  %d, %s\n", subs->callid.len, subs->callid.s);

}

void sendSubscribeNotify(const char **msg)
{
    subs_t subs;
    memset(&subs, 0, sizeof(subs_t));
    cJSON *root;
    root = cJSON_Parse(*msg);

    parseSubscription(&subs, &root);
    subs.event = &event;
/*
    LM_DBG("-------------------------------------\n");
    LM_DBG("callid  %d, %s\n", subs.callid.len, subs.callid.s);
    LM_DBG("local_contact %d, %s\n", subs.local_contact.len, subs.local_contact.s);
    LM_DBG("record_route %d, %s\n", subs.record_route.len, subs.record_route.s);
    LM_DBG("reason %d, %s\n", subs.reason.len, subs.reason.s);
    LM_DBG("event_id %d, %s\n", subs.event_id.len, subs.event_id.s);
    LM_DBG("local_cseq %d\n", subs.local_cseq);
    LM_DBG("remote_cseq %d\n", subs.remote_cseq);
    LM_DBG("status %d\n", subs.status);
    LM_DBG("version %d\n", subs.version);
    LM_DBG("expires %d\n", subs.expires);
    LM_DBG("contact %d, %s\n", subs.contact.len, subs.contact.s);
    LM_DBG("-------------------------------------\n");

    LM_DBG("%s\n", subs.event->name.s);
    LM_DBG("%s\n", subs.event->content_type.s);
*/
    if (notify(&subs, NULL, NULL, 0, NULL, 0) < 0) {
        LM_ERR("Failed to send notify request\n");

    }
    else
        LM_DBG("NOTIFY SENT\n");

    cJSON_Delete(root);
}

void sendPublishNotify(const char **key, char *pres_uri, char *etag)
{
//    LM_DBG("----------Presence URI-------------%s \n ",pres_uri);

    presentity_t presentity;
    memset(&presentity, 0, sizeof(presentity_t));
    str sender = {NULL, 0};
    sender.s = pres_uri;
    sender.len = strlen(pres_uri);
    str body = {NULL, 0};
    body.s = *key;
    body.len = strlen(*key);

    presentity.sender = &sender;
    presentity.event = &event;

    if (publ_notify(&presentity, sender, body.s ? &body : 0, NULL, NULL, NULL, 1) < 0) {
        LM_ERR("while sending notify\n");
    }
    LM_DBG("---------------NOTIFY SENT-----------------\n");

}

void parseKey(const char *key, str *callid, str *to_tag, str *from_tag)
{
    char *temp = key;
    int count = 0;

    //key == to_tag:from_tag:callid
    while (*key++ != ':')
        count++;
//    to_tag->s = pkg_malloc(sizeof(char) * count + 1);
    strncpy(to_tag->s, temp, count);
    to_tag->s[count] = '\0';
    to_tag->len = count;
    temp = key;
    count = 0;

    while (*key++ != ':')
        count++;
    //  from_tag->s = pkg_malloc(sizeof(char) * count + 1);
    strncpy(from_tag->s, temp, count);
    from_tag->s[count] = '\0';
    from_tag->len = count;
    temp = key;

    count = strlen(temp);
    //callid->s = pkg_malloc(sizeof(char) * count + 1);
    strncpy(callid->s, temp, count);
    callid->s[count] = '\0';
    callid->len = count;
}


//Old method. Used when subs key is passed in queue.
/*
void sendSubscribeNotify(const char *key, str *pres_uri, str *subs_uri) {
    subs_t subs;
    memset(&subs, 0, sizeof(subs_t));
    //str callid, to_tag, from_tag;

    subs.pres_uri = *pres_uri;

    char reason[50];
    char local_contact[50];
    char event_id[50];
    char record_route[50];
    char contact[50];

    subs.record_route.s = record_route;
    subs.contact.s = contact;
    subs.event_id.s = event_id;
    subs.local_contact.s = local_contact;
    subs.reason.s = reason;

    //   LM_DBG("Pres_URI: %s %d\n", subs.pres_uri.s, subs.pres_uri.len);

    if (fetchSubscriberFromCache(key, &subs) != 0)
    {
        LM_ERR("Nothing to do\n");
    } else
    {
        */
/*    LM_DBG("-------------------------------------\n");
            LM_DBG("local_contact %d, %s\n", subs.local_contact.len, subs.local_contact.s);
            LM_DBG("record_route %d, %s\n", subs.record_route.len, subs.record_route.s);
            LM_DBG("reason %d, %s\n", subs.reason.len, subs.reason.s);
            LM_DBG("event_id %d, %s\n", subs.event_id.len, subs.event_id.s);
            LM_DBG("local_cseq %d\n", subs.local_cseq);
            LM_DBG("remote_cseq %d\n", subs.remote_cseq);
            LM_DBG("status %d\n", subs.status);
            LM_DBG("version %d\n", subs.version);
            LM_DBG("expires %d\n", subs.expires);
            LM_DBG("contact %d, %s\n", subs.contact.len, subs.contact.s);
            LM_DBG("-------------------------------------\n");
        *//*

        char f_user[pres_uri->len];
        char f_domain[pres_uri->len];
        subs.from_user.s = f_user;
        subs.from_domain.s = f_domain;
        splitUserDomain(pres_uri, &subs.from_user, &subs.from_domain);
        //    LM_DBG("From User: %s %s\n", subs.from_user.s, subs.from_domain.s);
        char t_user[subs_uri->len];
        char t_domain[subs_uri->len];
        subs.to_user.s = t_user;
        subs.to_domain.s = t_domain;

        splitUserDomain(subs_uri, &subs.to_user, &subs.to_domain);
        //      LM_DBG("to User: %s %s\n", subs.to_user.s, subs.to_domain.s);

        //      LM_DBG("to User: %s %s\n", subs.to_user.s, subs.to_domain.s);

        char callid[50], to_tag[50], from_tag[50];
        subs.callid.s = callid;
        subs.to_tag.s = to_tag;
        subs.from_tag.s = from_tag;


        parseKey(key, &subs.callid , &subs.to_tag, &subs.from_tag);


        //     LM_DBG("Subs fetched\n");
        */
/*pres_ev_t *event = pkg_malloc(sizeof(pres_ev_t));
        memset(event, 0, sizeof(pres_ev_t));
        event->name.s = "presence"; // #define
        event->name.len = 8;// #define
        event->content_type.s = "application/pidf+xml";
        event->content_type.len = strlen("application/pidf+xml");*//*

        //      LM_DBG("Event created\n");
        subs.event = &event;
        //  LM_DBG("%s\n",subs.event->name.s);
        //  LM_DBG("%s\n",subs.event->content_type.s);
        //LM_DBG("Sending Notify TO: %s\n",subs_uri->s);
        //LM_DBG("Sending Notify FROM: %s\n",pres_uri->s);
        if (notify(&subs, NULL, NULL, 0, NULL, 0) < 0) {
            LM_ERR("Failed to send notify request\n");

        } else
            LM_DBG("NOTIFY SENT\n");
        // pkg_free(event);
*/
/*
        pkg_free(callid.s);
        pkg_free(to_tag.s);
        pkg_free(from_tag.s);
*//*

    }
}*/
//old method. Used when key is passed in queue.
/*

void sendPublishNotify(const char *key, char *pres_uri, char *etag) {
//    LM_DBG("----------Presence URI-------------%s \n ",pres_uri);
//    LM_DBG("---------------etag----------------%s \n ",etag);

    presentity_t presentity;
    memset(&presentity, 0, sizeof(presentity_t));
    str *sender = NULL;
    sender = (str *) pkg_malloc(sizeof(str));
    sender->s = pres_uri;
    sender->len = strlen(pres_uri);
    */
/* pres_ev_t *event = pkg_malloc(sizeof(pres_ev_t));
     memset(event, 0, sizeof(pres_ev_t));
     event->name.s = "presence";
     event->name.len = 8;
     event->content_type.s = "application/pidf+xml";
     event->content_type.len = strlen("application/pidf+xml");

 *//*

    str e = str_init(etag);
    presentity.etag = e;
    presentity.sender = sender;
    presentity.event = &event;

*/
/*
    LM_DBG("---------------Presentity-----------------\n");
    LM_DBG("%s\n",presentity.event->name.s);
    LM_DBG("%s\n",presentity.etag.s);
    LM_DBG("%s\n",presentity.sender->s);
*//*


    if (fetchSinglePresentityFromCache(key, &presentity) != 0) {
        LM_ERR("Nothing to do\n");
    } else {
        //   if (presentity.extra_hdrs)
        //    LM_DBG("%s\n", presentity.extra_hdrs->s);

        // LM_DBG("---------------COMPLETE-----------------\n");
        if (publ_notify(&presentity, *sender, presentity.body.s ? &presentity.body : 0,
                        &presentity.etag, NULL, NULL, 1) < 0) {
            LM_ERR("while sending notify\n");

        }
        LM_DBG("---------------NOTIFY SENT-----------------\n");
    }
    pkg_free(sender);
    //pkg_free(event);
}*/
/*
 * Used to read msg in form of sip:a@y.com:presence:etag
 * */
/*
void handlePublishMsg(const char *msg) {
    // msg == sip:a@y.com:presence:etag
    //LM_DBG("Received msg: %s\n", msg);

    int count = 4; // skip sip:
    while (msg[count] != ':') {
        count++;
    }
    char pres_uri[count+1];
    //char *pres_uri = pkg_malloc(sizeof(char) * count);
    strncpy(&pres_uri, msg, 4 + count);
    pres_uri[count++] = '\0'; // count++ to move skip :

    //loop to substring etag
    while (msg[count] != ':') {
        count++;
    }

    char *etag = msg + count + 1; // +1 to skip :.
    sendPublishNotify(msg, &pres_uri, etag);
    //pkg_free(pres_uri);
}
*/

/*void handleSubscribeMsg(const char *msg) {
    //msg == pres_uri+subs_uri+key
    //LM_DBG("Received msg: %s\n", msg);
    int count = 0;
    char *key = msg;
    while (*msg++ != '+') // find pres_uri
        count++;
    char presuri[count+1];
    str pres_uri, subs_uri;
   // LM_DBG("Received msg: %d-%d-%d\n", sizeof(char), count, sizeof(char) * count);

    pres_uri.s=presuri;
    strncpy(pres_uri.s, key, count);
    pres_uri.len = count;
    pres_uri.s[count] = '\0';

    count = 0;
   // LM_DBG("Pres_URI: %s\n", pres_uri.s);
    key = msg; //skip +
    while (*msg++ != '+') // find subs_uri
        count++;
    char subsuri[50];

    //subs_uri.s = pkg_malloc(sizeof(char) * count + 1);
    subs_uri.s=subsuri;
    strncpy(subs_uri.s, key, count);
    subs_uri.len = count;
    subs_uri.s[count++] = '\0';


//    LM_DBG("subs_uri: %s\n", subs_uri.s);
    key = msg; // skip +
 //   LM_DBG("Key: %s\n", key);

    sendSubscribeNotify(key, &pres_uri, &subs_uri);
//    pkg_free(pres_uri.s);
//    pkg_free(subs_uri.s);
}*/