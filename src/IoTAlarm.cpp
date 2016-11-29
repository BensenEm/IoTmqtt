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
#define PUBCLIENT    "PubCoord"
#define SUBCLIENT    "SubCoord"
#define QOS         1
#define TIMEOUT     10000L
//#define USER		"snfdsmlk"				//USER & PW for m21.cloudmqtt.com:12765
//#define PW		"iK32pQD1FXDA"
#define ADDRESS     "pascal-stuedlein.de:62454"

//Datatype-Struct passed to MQTTHandler consisting of a topic String and a message String
	struct MqttPckg{
		std::string topic;
		std::string mssg;
		MqttPckg(){};
		MqttPckg(std::string t, std::string m){
			topic=t;
			mssg=m;
		}
	};

//Queues handling incoming data, and outgoing data
	std::queue<MqttPckg> inQueue;
	std::queue<MqttPckg> outQueue;

// Global subscriber and publisher Clients
	MQTTClient subClient;
	MQTTClient pubClient;

	int waitingIntervall=10000;

//Callback is activated when Message delivered
	void delivered(void *context, MQTTClient_deliveryToken dt){
		printf("Message with token value %d delivery confirmed\n", dt);
		//deliveredtoken = dt;
	}
//Callback is activated when Message arrived. Pushes Message into inQueue
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

//Callback is activated when connection is lost
	void connlost(void *context, char *cause){
		printf("\nConnection lost\n");
		printf("     cause: %s\n", cause);
	}

// Initiates a Subscribing Client
	MQTTClient initSubscriber(char* topic, char* subClient){
		int rc;
		MQTTClient client;
		MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
		MQTTClient_create(&client, ADDRESS, SUBCLIENT, MQTTCLIENT_PERSISTENCE_NONE, NULL);
		conn_opts.keepAliveInterval = 20;
		conn_opts.cleansession = 1;
//		conn_opts.username = USER;
//		conn_opts.password = PW;


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

//Initiates a Publishing Client
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

//Disconnects and Destroys Clients
	void disconnectDestroy(MQTTClient client){
		MQTTClient_disconnect(client, 100);
		MQTTClient_destroy(&client);
	}

//Prints inQueue
	void printQueue(){
		while (!inQueue.empty()){
			std::string ms= inQueue.front().mssg;
			std::string to= inQueue.front().topic;
			inQueue.pop();
			std::cout<<to+" - "+ ms<<std::endl;
		}
		std::cout<<"End"<<std::endl;
	}
//Publishes a single Package. Be aware that the client who delivers package is hardcoded (pubClient) referring
//to the only publisher in this application
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
		}
	}




	int main() {
//start subscriber by passing topic and an identifier
		subClient= initSubscriber( (char*)"#", (char*) "s2" );
//start publisher by passing an identifier
		pubClient= initPublisher( (char*) "p1" );

//create Packages (stringTopic and stringMessage) and push them in the outQueue for testing purposes
		MqttPckg a("t1", "mess1");
		MqttPckg b("t2", "mess2");
		MqttPckg c("t3", "m3ess");
		outQueue.push(a);
		outQueue.push(b);
		outQueue.push(c);

//publishing Whatever is in the outQueue
		publishQueue();

//let Program run until pressed 'Q' or 'q'
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
