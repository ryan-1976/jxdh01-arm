//#include "stdio.h"
//#include "stdlib.h"
//#include "string.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MQTTClient.h"
#include <unistd.h>
#include "circlebuff.h"

//#define ADDRESS     "tcp://localhost:1883"
#define ADDRESS     "tcp://192.168.7.240:1883"
#define CLIENTID    "11111111111111pub"
#define CLIENTID1   "11111111111122pub"
#define TOPIC       "mqtt/11111111111111"
#define TOPIC1       "mqtt/11111111111122"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L
char pubBuf[2048];
extern MQTT_SENT_BUFF_T   mqSentBuff;

void *mqtt_pub_treat(int argc, char* argv[])
{
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc, i;

    //int MQTTClient_isConnected	(	MQTTClient 	handle	);



	printf("--22-----enter mqtt_pub_treat----------------- \n");
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("pub Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    pubmsg.payload = PAYLOAD;
    pubmsg.payloadlen = strlen(PAYLOAD);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

	while(1)
	{
		//printf("pub data----------------- %d\n");
		//MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
//		for(i =0;i<1024;i++)
//		{
//			pubBuf[i]= 3;
//		}
		//sleep(10);
		pthread_mutex_lock(&mqSentBuff.lock);
		pthread_cond_wait(&mqSentBuff.newPacketFlag, &mqSentBuff.lock);

		for(i =0;i<mqSentBuff.len;i++)
		{
			pubBuf[i]= mqSentBuff.data[i];
		}

		pthread_mutex_unlock(&mqSentBuff.lock);
		
		if(mqSentBuff.mqttTopicFlag == MQTPA)
		{
			MQTTClient_publish(client, TOPIC,mqSentBuff.len, pubBuf,QOS,0, &token);
		}
		else
		{
			MQTTClient_publish(client, TOPIC1,mqSentBuff.len, pubBuf,QOS,0, &token);
		}

		// printf("Waiting for up to %d seconds for publication of %s\n"
		// "on topic %s for client with ClientID: %s\n",
		// (int)(TIMEOUT/1000), PAYLOAD, TOPIC, CLIENTID);
		// rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
		 //printf("Message with delivery token %d delivered\n", token);

		//--------------------------------------
		//sleep(2);
		
	}

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    //return rc;
}
