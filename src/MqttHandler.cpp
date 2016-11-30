/*
 * MqttHandler.cpp
 *
 *  Created on: 29.11.2016
 *      Author: bensen
 */

#include "MqttHandler.h"
#include <stdexcept>
#include <string>
#include <queue>

MqttHandler::MqttHandler() {
	// TODO Auto-generated constructor stub

}

MqttHandler::~MqttHandler() {
	// TODO Auto-generated destructor stub
}

int getEndId(std::string macNId){
	for (std::vector<IdResolver>::iterator it = idTable.begin() ; it != idTable.end(); ++it){
		if (it->coorId == macNId)
			return it->endId;
	}
	return -99;
}
std::string getType(std::string macNId){
	for (std::vector<IdResolver>::iterator it = idTable.begin() ; it != idTable.end(); ++it){
		if (it->coorId == macNId)
			return it->type;
	}
	return "unknowm";
}
std::string getCoorId(int endId){
	for (std::vector<IdResolver>::iterator it = idTable.begin() ; it != idTable.end(); ++it){
		if (it->endId == endId)
			return it->coorId;
		}
	return "none";
}

void addIdEntry(std::string type, std::string coorId){
	int index= idTable.size();
	try{
		IdResolver n (idTable.at(index-1).endId+1, type, coorId);
		idTable.push_back(n);
	}
	catch(std::out_of_range& e){
		IdResolver n (1, type, coorId);
		idTable.push_back(n);

	}
}
int removeIdEntry(std::string coorId){
	for (std::vector<IdResolver>::iterator it = idTable.begin() ; it != idTable.end(); ++it){
		if(it->coorId==coorId){
			idTable.erase(it);
			return 1;
		}
	}
	return -1;
}
void printIdTable(){

	std::cout<<"========================================\nSensor ID Resolver: \nEndID: ID for Enddevices. \nCoorID = ID for Coordinator.\n------------------------------"<<std::endl;
	for (std::vector<IdResolver>::iterator it = idTable.begin() ; it != idTable.end(); ++it){
		std::cout<<"EndID:..."<<it->endId<<"   CoorID..."<<it->coorId<<"   Type:..."<<it->type<<std::endl;
	}
	std::cout<<"========================================"<<std::endl;
}


void delivered(void *context, MQTTClient_deliveryToken dt){
	printf("Message with token value %d delivery confirmed\n", dt);
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
	int i = 0;
	disconnectDestroy(subClient);
	disconnectDestroy(pubClient);
	while(i<10){
		//	nanosleep(100);
			i++;
		try{
			subClient= initSubscriber( (char*)"#", (char*) "s2" );
			pubClient= initPublisher( (char*) "p1" );
			break;
		}
		catch(int e){
			printf("%d",e);
		}
	}

}
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
		throw(-1);
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
		throw(-1);
	}
	return client;
}
void disconnectDestroy(MQTTClient client){
	MQTTClient_disconnect(client, 100);
	MQTTClient_destroy(&client);
}
void printInQueue(){
	while (!inQueue.empty()){
		std::string ms= inQueue.front().mssg;
		std::string to= inQueue.front().topic;
		inQueue.pop();
		std::cout<<to+" - "+ ms<<std::endl;
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

void publishOutQueue(){
	while(!outQueue.empty()){
		publish(outQueue.front());
		outQueue.pop();
	}
}

/*
 * @param methodType: 1(actual), 2(history), 3(battery), 4(alarm)
 * @param sensorType: unused Needed so far
 * @param coorId: keyId on Coordinator Side
 * @param *payload: Pointer to Vector of DataValues. With most recent value at front!
 *
 */

//MqttPckg buildMqttPckg(int mPckgType, int type, int mac, int id, std::string payload ){
MqttPckg buildMqttPckg(int methodType, int sensortype, std::string coorId, std::vector<std::string> * payload ){
	MqttPckg pckg;
	int endId= getEndId(coorId);
	std::string type= getType(coorId);
	auto prepareTopic = [&]() {
		pckg.topic="/response/";
		switch(methodType){
		case(1):
			pckg.topic+="actual/:";
			break;
		case(2):
			pckg.topic+="history/:";
			break;
		case(3):
			pckg.topic+="battery/:";
			break;
		case(4):
			pckg.topic+="alarm/:";
		default:
			break;
		}
		pckg.topic+=std::to_string(endId);
		pckg.topic+="/:";
		pckg.topic+= type;
		pckg.topic+="/";
	};
	switch (methodType){
	//actual
	case (1):
		prepareTopic();
		pckg.mssg=payload->front();
		break;
	//history
	case (2):
		prepareTopic();
		for (std::vector<std::string>::iterator it = payload->begin() ; it != payload->end(); ++it){
			pckg.mssg+=*it;
			pckg.mssg+="|";
		}
		break;
	//battery:
	case (3):
		prepareTopic();
		pckg.mssg=payload->front();
		break;
	//alarm:
	case (4):
		prepareTopic();
		pckg.mssg="ALARM";
		break;
	}
	return pckg;
}

int main (){
	//start subscriber by passing topic and an identifier
			subClient= initSubscriber( (char*)"#", (char*) "s2" );
	//start publisher by passing an identifier
			pubClient= initPublisher( (char*) "p1" );
	//define Ids in IdResolver
//			idTable.push_back(IdResolver(0, "temp", "mac2784728"));
//			idTable.push_back(IdResolver(1, "temp", "mac4728345"));
			addIdEntry("temp", "mac3255");
			addIdEntry("humid", "mac3345");
			addIdEntry("accel", "mac3245");
			addIdEntry("cam", "mac3243");
			printIdTable();
			removeIdEntry("mac3245");
			addIdEntry("newKid", "mac5643");
			printIdTable();
			std::vector<std::string> *dataHistory= new std::vector<std::string>;
			dataHistory->push_back("valOne");
			dataHistory->push_back("valTwo");
			dataHistory->push_back("valThree");

	//create Packages (stringTopic and stringMessage) and push them in the outQueue for testing purposes
			MqttPckg a("t1", "m1");
			MqttPckg b("t2", "m2");
			MqttPckg c("t3", "m3");
			outQueue.push(a);
			outQueue.push(b);
			outQueue.push(c);
			outQueue.push(buildMqttPckg(4,33,"mac5643", dataHistory));


	//publishing Whatever is in the outQueue
			publishOutQueue();

	//let Program run until pressed 'Q' or 'q'
			int ch;
			do{
				ch = getchar();
			} while (ch!='Q' && ch != 'q');

	//print Messages arrived and close Subscriber
			printInQueue();

			//	disconnectDestroy(aSubClient);
			disconnectDestroy(subClient);
			disconnectDestroy(pubClient);
	return 0;
}
