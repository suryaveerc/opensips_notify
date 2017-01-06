//
// Created by suryaveer on 2016-10-04.
//

#include "../../dprint.h"
#include "queue.h"
#include "queue_msg_handler.h"

static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure);

char  subj[15];
char  queueGroupName[15];
natsOptions      *options   = NULL;
natsConnection  *connection  = NULL;
natsStatus      status=NATS_OK;
natsSubscription    *subs   = NULL;

int queue_connect(const char  **serverUrls) {
    if (natsOptions_Create(&options) != NATS_OK)
        status = NATS_NO_MEMORY;

    LM_DBG("In queueconnect*********\n");
    LM_DBG("%s\n", serverUrls[0]);

    if (status == NATS_OK)
        status = natsOptions_SetServers(options, (const char **) serverUrls, 1);

    status = natsConnection_Connect(&connection, options);

    if (status == NATS_OK)
    {
        LM_DBG("Connection..\n");
        return 0;
    }
    else
    {
        LM_DBG("No Connection..\n");
        return -1;
    }
}
int setQueue( const char* subject)
{
    LM_DBG("*********************** In setQueue %s...........\n", subject);
    //subj = malloc(sizeof(char) * strlen(subject) + 1);

    strcpy(subj,subject);
    LM_DBG("Subject is %s \n", subj);
    return 0;
}
int publish_msg(const char* txt){

    status = natsConnection_PublishString(connection, subj, txt);
    if (status == NATS_OK)
    {
        LM_DBG("Msg sending.. %s \n",txt);
        return 0;
    }
    else
    {
        LM_DBG("Error: %d - %s\n", status, natsStatus_GetText(status));
        nats_PrintLastErrorStack(stderr);
        return -1;
    }
    /*if (s == NATS_OK)
        s = natsConnection_FlushTimeout(conn, 1000);
    else
        LM_DBG("Not flushing..");*/
}

void queue_disconnect()
{
    // Destroy all our objects to avoid report of memory leak
    natsSubscription_Destroy(subs);
    natsConnection_Destroy(connection);
    natsOptions_Destroy(options);

    // To silence reports of memory still in used with valgrind
    nats_Close();
}
int subscribeQueue(const char* subject, const char* queueGroup)
{
    if (status == NATS_OK) {
        status = natsConnection_QueueSubscribe(&subs, connection, subject, queueGroup, onMsg, NULL);
        //status = natsConnection_QueueSubscribeSync(&subs, connection, subject, queueGroup);
        status = natsSubscription_SetPendingLimits(subs, -1, -1);

    }else {
        LM_DBG("Unable to subscribe to queue\n");
    }
    return status;

}
static void onMsg(natsConnection *nc, natsSubscription *sub, natsMsg *msg, void *closure)
{
    char *m = natsMsg_GetData(msg);

    //if(sub->queue)

  //  LM_DBG("Queue Group %s \n",natsMsg_GetSubject(msg));
    LM_DBG("Message %s \n",natsMsg_GetData(msg));
    if(strcmp(SUBJ_PUBLISH,natsMsg_GetSubject(msg))==0)
        handlePublishMsg(m);
    else if(strcmp(SUBJ_SUBSCRIBE,natsMsg_GetSubject(msg))==0)
        handleSubscribeMsg(m);
    natsMsg_Destroy(msg);
    return 0;
}

