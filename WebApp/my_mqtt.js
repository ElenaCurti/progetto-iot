var HOST = "192.168.1.53"
var PORT = 8000;
var TOPIC_IN = "immagine";
var userId = "elenaId-webapp-";
var passwordId ="";

function print_message(messaggio){
    document.getElementById("messages").innerHTML += "<span>" + messaggio +"</span><br>"; 
}

function startConnect(){
    
    try{
        if (client.isConnected())
            return;
    } catch (err){}

    userId += parseInt(Math.random() * 100);

    print_message("Connecting to " + HOST + " on port " +PORT);
    print_message("Using the client Id " + userId );

    client = new Paho.MQTT.Client(HOST,Number(PORT),userId);
    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;

    client.connect({
        onSuccess: onConnect,
        // TODO implementa username e password
        userName: userId
 //       passwordId: passwordId
    });

    init_canvas();



}



function onConnect(){
    print_message("Subscribing to topic "+ TOPIC_IN);
    client.subscribe(TOPIC_IN);
}



function onConnectionLost(responseObject){
    print_message("ERROR: Connection is lost.");
    if(responseObject !=0){
        print_message("ERROR:"+ responseObject.errorMessage );
    }
}

function onMessageArrived(message){
    console.log("OnMessageArrived: "+message.payloadString.slice(0,20));
    print_message("Topic:"+message.destinationName+" | Message : "+message.payloadString.slice(0,20) );
    pixel = hexStringToUint8Array(message.payloadString);
    display(pixel);
}

function startDisconnect(){
    client.disconnect();
    print_message("Disconnected.");
}

function publishMessage(id_button_premuto){

    // startConnect();

    console.log("button_premuto: " + id_button_premuto);

    msg = "apri "+id_button_premuto ; // TODO da decidere 

    Message = new Paho.MQTT.Message(msg);
    Message.destinationName = TOPIC_OUT;

    client.send(Message);
    print_message("Message to topic "+TOPIC_OUT+" is sent ");


}
