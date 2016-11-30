/*
 * MqttHandler.h
 *
 *  Created on: 29.11.2016
 *      Author: bensen
 */

#ifndef MQTTHANDLER_H_
#define MQTTHANDLER_H_

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <iostream>
#include <queue>
#include <vector>
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

//Datatype for IDs
struct IdResolver{
	int endId;
	std::string type;
	std::string coorId;
	IdResolver(){
		endId= -1; type="none"; coorId="none";
	}
	IdResolver(int oI, std::string t, std::string mI){
		endId= oI; type=t; coorId=mI;
	}

};

//Vector ID-Table and Functions
	std::vector<IdResolver> idTable;
	int getOutId(std::string coorId);
	std::string getMacAndInId(std::string endId);
	void addIdEntry(std::string type, std::string coorId);


//Queues handling incoming data, and outgoing data
	std::queue<MqttPckg> inQueue;
	std::queue<MqttPckg> outQueue;

// Global subscriber and publisher Clients
	MQTTClient subClient;
	MQTTClient pubClient;
//Callback is activated when Message delivered
	void delivered(void *context, MQTTClient_deliveryToken dt);

//Callback is activated when Message arrived. Pushes Message into inQueue
	int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);

//Callback is activated when connection is lost
	void connlost(void *context, char *cause);

// Initiates a Subscribing Client
	MQTTClient initSubscriber(char* topic, char* subClient);

//Initiates a Publishing Client
	MQTTClient initPublisher(char* pubClient);

//Disconnects and Destroys Clients
	void disconnectDestroy(MQTTClient client);

//Prints inQueue
	void printInQueue();

//Publishes a single Package. Be aware that the client who delivers package is hardcoded (pubClient) referring
//to the only publisher in this application
	void publish(MqttPckg p);

//Publishes everything within the outQueue
	void publishOutQueue();

//Builds a Package ready for publishing from a set of Values
	MqttPckg buildMqttPckg(int mPckgType, int type, int mac, int id, std::string payload );


inline void operator()(){
	//start subscriber by passing topic and an identifier
	subClient= initSubscriber( (char*)"#", (char*) "s2" );
	//start publisher by passing an identifier
	pubClient= initPublisher( (char*) "p1" );
	while(true){
		publishOutQueue();
	}
	disconnectDestroy(subClient);
	disconnectDestroy(pubClient);
}


class MqttHandler {
public:
	MqttHandler();
	virtual ~MqttHandler();
};

#endif /* MQTTHANDLER_H_ */
