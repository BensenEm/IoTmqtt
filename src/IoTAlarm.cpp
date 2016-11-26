//============================================================================
// Name        : IoTAlarm.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
#include <queue>
#include "MQTTClient.h"
//#include "MQTTHandler.h"
#define PUBCLIENT    "PubCoord"
#define SUBCLIENT    "SubCoord"
#define QOS         1
#define TIMEOUT     10000L
#define USER		"snfdsmlk"
#define PW			"iK32pQD1FXDA"
#define ADDRESS     "pascal-stuedlein.de:62454"

struct MqttPckg{
	std::string topic;
	std::string mssg;
	MqttPckg(){};
	MqttPckg(std::string t, std::string m){
		topic=t;
		mssg=m;
	}
};
std::queue<MqttPckg> inQueue;
std::queue<MqttPckg> outQueue;
MQTTClient subClient;
MQTTClient pubClient;
int waitingIntervall=10000;
void delivered(void *context, MQTTClient_deliveryToken dt){
	printf("Message with token value %d delivery confirmed\n", dt);
	// deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message){
	int i;
	char* payloadptr;
	MqttPckg pack;
	pack.topic=topicName;
	pack.mssg=" ";
	payloadptr = (char*) message->payload;
	for(i=0; i<message->payloadlen; i++){
		pack.mssg+=*payloadptr++;
	}
	inQueue.push(pack);
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);

	return 1;
}

void connlost(void *context, char *cause){
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
}

MQTTClient initSubscriber(char* topic, char* subClient){
	int rc;
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_create(&client, ADDRESS, "subClient", MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.username = USER;
	conn_opts.password = PW;


	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(-1);
	}
	printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
			"Press Q<Enter> to quit\n\n", topic, SUBCLIENT, QOS);
	MQTTClient_subscribe(client, topic, QOS);
	return client;
}

MQTTClient initPublisher(char* pubClient){
	int rc;
	MQTTClient client;
	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_create(&client, ADDRESS, "pubClient", MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
//	conn_opts.username = USER;
//	conn_opts.password = PW;
	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(-1);
	}
	return client;
}


void disconnectDestroy(MQTTClient client){
	MQTTClient_disconnect(client, 100);
	MQTTClient_destroy(&client);
}

void printQueue(){
	while (!inQueue.empty()){
		std::string ms= inQueue.front().mssg;
		std::string to= inQueue.front().topic;
		inQueue.pop();
		std::cout<<to+" "+ ms<<std::endl;
	}
	std::cout<<"End"<<std::endl;
}

void publish(MqttPckg p){
	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	const char* m= p.mssg.c_str();
	const char* t= p.topic.c_str();
	pubmsg.payload = (void*) m;
	pubmsg.payloadlen = strlen((char*)pubmsg.payload);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	MQTTClient_deliveryToken token;
	const char* topicC= t;
	MQTTClient_publishMessage(pubClient, topicC, &pubmsg, &token);
//	std::cout<<p.mssg +"  "+p.topic<<std::endl;
}

void publishQueue(){
	while(!outQueue.empty()){
		publish(outQueue.front());
		outQueue.pop();
		std::cout<<"ok"<<std::endl;
	}
	wait(&waitingIntervall);
}




int main() {
	//start subscribers by passing topic and a identifier
//	MQTTClient aSubClient= initSubscriber( (char*)"t1", (char*) "s1" );
	subClient= initSubscriber( (char*)"#", (char*) "s2" );
	pubClient= initPublisher( (char*) "p1" );

	MqttPckg a("t1", "mess1");
	MqttPckg b("t2", "mess2");
	MqttPckg c("t3", "m3ess");
	outQueue.push(a);
	outQueue.push(b);
	outQueue.push(c);
//

	//publishing
	publishQueue();
//	publish(c);

	//let everything run until pressed 'Q' or 'q'
	int ch;
	do{
		ch = getchar();
	} while (ch!='Q' && ch != 'q');

	//print Messages arrived and close Subscriber
	printQueue();
//	disconnectDestroy(aSubClient);
	disconnectDestroy(subClient);
	disconnectDestroy(pubClient);
}
