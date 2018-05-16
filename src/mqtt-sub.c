#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"
#include "circlebuff.h"
#include <unistd.h>

//#define ADDRESS     "tcp://localhost:1883"
//#define ADDRESS     "tcp://192.168.3.101:1883"
#define ADDRESS     "tcp://182.61.43.75:1883"
#define CLIENTID    "11111111111111sub"
#define CLIENTID1   "11111111111122sub"
#define TOPIC       "mqtt/31111111111111"
#define TOPIC1       "mqtt/31111111111122"
#define PAYLOAD     "Hello World!"
#define QOS         1
#define TIMEOUT     10000L

extern DATAS_BUFF_T   comBuff0; 

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
int msgarrvd1(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf(" new mqtt Message arrived:");
    printf(" topic: %s", topicName);
	printf(" topicLen:%d\n ",message->payloadlen);

    payloadptr = message->payload;
	
	pthread_mutex_lock(&comBuff0.lock);

	AP_circleBuff_WritePacket(payloadptr++,message->payloadlen,MQTPA2DTU);
	pthread_cond_signal(&comBuff0.newPacketFlag);
	pthread_mutex_unlock(&comBuff0.lock);
	
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf(" new mqtt Message arrived:");
    printf(" topic: %s", topicName);
	printf(" topicLen:%d\n ",message->payloadlen);

    payloadptr = message->payload;

	pthread_mutex_lock(&comBuff0.lock);

	AP_circleBuff_WritePacket(payloadptr++,message->payloadlen,MQTPA2DTU);
	pthread_cond_signal(&comBuff0.newPacketFlag);
	pthread_mutex_unlock(&comBuff0.lock);
	
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

void *mqtt_sub_treat(int argc, char* argv[])
{
    MQTTClient client,client1;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

	//--------------------------------------------------------------
	//printf("-------enter mqtt_sub_treat-----------------\n");
    MQTTClient_create(&client, ADDRESS, CLIENTID,
    MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 60;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("sub start up Failed to connect, return code %d\n", rc);
        //exit(EXIT_FAILURE);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);
//--------------------------------------------------------------
//    MQTTClient_create(&client1, ADDRESS, CLIENTID1,
//        MQTTCLIENT_PERSISTENCE_NONE, NULL);
//    conn_opts.keepAliveInterval = 20;
//    conn_opts.cleansession = 1;
//
//    MQTTClient_setCallbacks(client1, NULL, connlost, msgarrvd1, delivered);
//
//    if ((rc = MQTTClient_connect(client1, &conn_opts)) != MQTTCLIENT_SUCCESS)
//    {
//        printf("Failed to connect, return code %d\n", rc);
//        exit(EXIT_FAILURE);
//    }
//    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
//           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID1, QOS);
//    MQTTClient_subscribe(client1, TOPIC1, QOS);
//--------------------------------------------------------------

    do 
    {
		while(!MQTTClient_isConnected(client))
		{
			if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
			{
				printf(" sub retry to connect failure, return code %d\n", rc);
			}
			else
			{
				printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
				           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
				    MQTTClient_subscribe(client, TOPIC, QOS);
			}
			sleep(3);
			printf("-------sub wait to connected-----------------\n");
		};
		sleep(3);

    }while(1);

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    //return rc;
}
