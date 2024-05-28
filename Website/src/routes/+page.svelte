<script>
    import mqtt from "mqtt";
    import { onMount } from 'svelte';

    let client;

    let thumb;
    var motorSpeed = 100;

    var lastDirection = -1; // Inte 1 eller 0
    let uploadDelay;
    var hasExited = false;

    function uploadMovement(boolean1, boolean2, integer){
        console.log(boolean1, boolean2, integer)
        // Motor (0) or Servo (1)
        // If motor, back (0) or front (1)
        // Motor speed (0 - 255), or servo rotation (0 - 180)

        const byte1 = (boolean1 << 1) | boolean2;
        const byte2 = integer;

        const buffer = new Uint8Array([byte1, byte2]);

        client.publish('ian.baldelli@gmail.com/movement', buffer, (err) => {
            if (err) {
                console.error('Failed to publish message:', err);
            } else {
                console.log('Message published');
            }
        });
    }

    function updateMovement(x, y, leveledX, leveledY, distance, angle){

       if (x == 0 && y == 0 ){ // Körs om man släpper joysticken

        uploadMovement(0,0,0)
        hasExited = true;
        return;
       }

       clearTimeout(uploadDelay)
       let rotatationValue = Math.round(-180 * angle/3.1416)


       let date = new Date();
       let direction;

       if (rotatationValue >  0){
            direction = 1;
            rotatationValue = -1 * rotatationValue + 180
       }
       else {
            direction = 0;
            rotatationValue = rotatationValue + 180
       }

        if (direction != lastDirection || hasExited){
            uploadMovement(0, direction, motorSpeed);
            hasExited = false;
        }

        lastDirection = direction
        uploadDelay = setTimeout(uploadMovement, 20, 1, 0, rotatationValue); // Kommer bara köra om det har gått 0.5 secunder från sista joystickrörelsen. (För att undvika att skicka för många paket till espn.)


    }

    function moveThumb(event){
        let clientY;
        const sliderRect = thumb.parentElement.getBoundingClientRect();

        if (event.type.includes('click')){
            clientY = event.clientY - sliderRect.top;
        }
        else{
            clientY = event.touches[0].clientY - sliderRect.top;
        }
        
        let ratioY = clientY / sliderRect.height;
        motorSpeed = -255 * ratioY + 255
        console.log(motorSpeed)
        console.log(ratioY)
        if (ratioY < 0){
            let newTop = (100 * 0).toString() + "%";
            thumb.style.top = newTop;
        }
        else {
            let newTop = (100 * ratioY).toString() + "%";
            thumb.style.top = newTop;
        }

        uploadMovement(0, lastDirection , motorSpeed);
    }

    function createJoystick(){
        import("joystick-controller").then(({ default: JoystickController }) => {
            new JoystickController(
                {
                    maxRange: 70,
                    level: 10,
                    radius: 70,
                    joystickRadius: 50,
                    opacity: 0.5,
                    containerClass: "joystick-container",
                    controllerClass: "joystick-controller",
                    joystickClass: "joystick",
                    distortion: true,
                    dynamicPosition: true,
                    dynamicPositionTarget: document.getElementById("zoneJoystick"),
                    mouseClickButton: "ALL",
                    hideContextMenu: true,
                },
                ({ x, y, leveledX, leveledY, distance, angle }) => updateMovement(x, y, leveledX, leveledY, distance, angle)
            );
        });
    }

    function setUpMQTT(){
        const MQTT_BROKER = "maqiatto.com";
        const MQTT_BROKER_PORT = 8883;
        const MQTT_USERNAME = "ian.baldelli@gmail.com";
        const MQTT_KEY = "adrenaline123";

        const options = {
            port: MQTT_BROKER_PORT,
            protocol: 'ws',
            username: MQTT_USERNAME,
            password: MQTT_KEY
        };

        client = mqtt.connect(`ws://${MQTT_BROKER}`, options);

        client.on('connect', () => {
            console.log('Connected to MQTT broker');

            client.subscribe('ian.baldelli@gmail.com/movement', (err) => {
                if (err) {
                    console.error('Failed to subscribe:', err);
                }
            });

            client.subscribe('ian.baldelli@gmail.com/sensor', (err) => {
                if (err) {
                    console.error('Failed to subscribe:', err);
                }
            });

        });

        client.on('message', (topic, message) => {
            //console.log(`Received message on topic ${topic}: ${message.toString()}`);
        });

        client.on('error', (err) => {
            console.error('Connection error:', err);
        });

        client.on('close', () => {
            console.log('Connection closed');
        });
    }

    onMount(() => {
        
        createJoystick()
        setUpMQTT()
        
    });
</script>

<svelte:head>
    <title>Home</title>
    <meta name="description" content="Svelte demo app" />
</svelte:head>

<img id="racing" src="racing.png" />
<div class="slider"  on:touchstart={moveThumb}
  on:touchmove={moveThumb}
  on:click={moveThumb}>
    <span class="value top">255</span>
    <span class="value middle">120</span>
    <div class="thumb" bind:this={thumb}></div>
</div>

<div id="zoneJoystick"></div>

<style>
    .slider {
        position: absolute;
        background: radial-gradient(circle, rgba(233, 233, 233, 0.3) 0%, rgba(233, 233, 233, 1) 100%);
        width: 78px;
        height: 90%;
        margin-top: auto;
        margin-bottom: auto;
        top: 0;
        left: 8%;
        bottom: 0;
        right: 0;
        border-radius: 30px;
        border: 2px solid #c4c4c4;
        z-index: 10;
    }
    #racing {
        position: absolute;
        width: 100%;
        height: auto;
        top: -75%;
    }
    .thumb {
        position: absolute;
        background: radial-gradient(circle, rgba(0, 0, 0, 0.36456589471726186) 0%, rgba(56, 56, 56, 0.41218494233630953) 27%);
        width: 78px;
        height: 78px; /* responsive thumb size */
        left: -0px;
        border-radius: 30px;
        transform: translateY(-50%); /* center the thumb */
    }
    #zoneJoystick {
        position: absolute;
        right: 0;
        width: 50%;
        height: 100%;
        top: 0;
    }
    .value {
        border-bottom: 2px solid #c4c4c4;
        color: #686868;
        padding: 1px;
        padding-left: 10px;
        padding-right: 14px;
        position: absolute;
    }
    .top {
        top: 12px;
    }
    .middle {
        top: 50%;
    }
    :global(.joystick-container) {
        z-index: 1 !important;
    }
</style>
