<script>
import mqtt from "mqtt";
import { onMount } from 'svelte';

let numberOfPlayers = 1; // Standard för antalet spelare är 1, om spelarna inte väljer något annat.

let players = [];
let playerTimes = [];

let client;

let thumb;
let hasExited = false;

let motorSpeed = 100;
let lastDirection = -1; // Får inte vara 1 eller 0

let uploadDelay;
let startTimerDelay;
let stopTimerDelay;
let raceTimer;
let playerHasStarted = false;

// Funktion för att ladda upp rörelse
function uploadMovement(isMotor, isForward, value) {
  console.log(isMotor, isForward, value); // Motor (0) eller Servo (1) // Om motor, bak (0) eller fram (1) // Motorhastighet (0 - 255), eller servorotation (0 - 180)

  // Komprimera till två bytes.
  const byte1 = (isMotor << 1) | isForward;
  const byte2 = value;
  const buffer = new Uint8Array([byte1, byte2]);

  client.publish('ian.baldelli@gmail.com/movement', buffer, (err) => {
    if (err) {
      console.error('Failed to publish message:', err);
    } else {
      console.log('Message published');
    }
  });
}

// Funktion för att uppdatera rörelse
function updateMovement(x, y, leveledX, leveledY, distance, angle) {
  if (x === 0 && y === 0) {
    // Körs om man släpper joysticken
    uploadMovement(0, 0, 0);
    hasExited = true;
    return;
  }

  clearTimeout(uploadDelay);

  let rotationValue = Math.round(-180 * angle / 3.1416);
  let direction;

  if (rotationValue > 0) {
    direction = 1;
    rotationValue = -rotationValue + 180;
  } else {
    direction = 0;
    rotationValue = rotationValue + 180;
  }

  if (direction!== lastDirection || hasExited) {
    uploadMovement(0, direction, motorSpeed);
    hasExited = false;
  }

  lastDirection = direction;
  uploadDelay = setTimeout(uploadMovement, 20, 1, 0, rotationValue); // Kommer bara köra om det har gått 0.5 sekunder från sista joystickrörelsen. (För att undvika att skicka för många paket till espn.)
}

// Funktion för att hantera tumrörelse
function moveThumb(event) {
  let clientY;
  const sliderRect = thumb.parentElement.getBoundingClientRect();

  if (event.type.includes('click')) {
    clientY = event.clientY - sliderRect.top;
  } else {
    clientY = event.touches[0].clientY - sliderRect.top;
  }

  let ratioY = clientY / sliderRect.height;
  motorSpeed = -255 * ratioY + 255;
  console.log(motorSpeed);
  console.log(ratioY);

  if (ratioY < 0) {
    let newTop = (100 * 0).toString() + "%";
    thumb.style.top = newTop;
  } else {
    let newTop = (100 * ratioY).toString() + "%";
    thumb.style.top = newTop;
  }

  if (!hasExited) {
    uploadMovement(0, lastDirection, motorSpeed);
  }
}

// Funktion för att starta en timer för en spelare
function startTimer(playerIndex) {
  playerHasStarted = true;
  const startTime = Date.now();

  raceTimer = setInterval(() => {
    const elapsedTime = Date.now() - startTime;
    const minutes = Math.floor(elapsedTime / 60000);
    const seconds = Math.floor((elapsedTime % 60000) / 1000);

    players[playerIndex] = `Player ${playerIndex + 1} : ${String(minutes).padStart(2, '0')}:${String(seconds).padStart(2, '0')}`;
    playerTimes[playerIndex] = minutes * 60 + seconds;
  }, 1000);
}

// Funktion för att stoppa en timer och spara den passerade tiden till en array
function stopTimer(playerIndex) {
  playerHasStarted = false;
  clearInterval(raceTimer);
  console.log(playerTimes);
}

// Funktion för att hantera mottagna MQTT-meddelanden
function recievedMQTT(message) {
  if (message.length === 1) {
    const byteValue = message[0];

    if (byteValue == 1) {
      // Första spelaren med tiden 0
      const playerIndex = playerTimes.findIndex(value => value === 0);

      if (playerHasStarted) {
        clearTimeout(stopTimerDelay);
        stopTimerDelay = setTimeout(stopTimer, 500, playerIndex);
      } else {
        clearTimeout(startTimerDelay);
        startTimerDelay = setTimeout(startTimer, 500, playerIndex);
      }
    }
  } else {
    console.error('Received message is not a single byte');
  }
}

// Funktion för att ladda ledartavlan
function loadLeaderboard() {
  const urlParams = new URLSearchParams(window.location.search);

  if (urlParams.has('players')) {
    numberOfPlayers = parseInt(urlParams.get('players'));
  }

  // Initiera spelarna array med en längd av numberOfPlayers
  players = Array(numberOfPlayers).fill().map((_, index) => {
    // För varje element, returnera en sträng som representerar spelaren
    const playerNumber = index + 1;
    return `Player ${playerNumber} : ---`;
  });

  playerTimes = Array(numberOfPlayers).fill().map(() => {
    // För varje element, returnera 0
    return 0;
  });
}

// Funktion för att skapa joystick
function createJoystick() {
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

// Funktion för att ställa in MQTT
function setUpMQTT() {
  const MQTT_BROKER = "maqiatto.com";
  const MQTT_BROKER_PORT = 8883;
  const MQTT_USERNAME = "ian.baldelli@gmail.com";
  const MQTT_KEY = "";

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
    if (topic == "ian.baldelli@gmail.com/sensor") {
      recievedMQTT(message);
    }
  });

  client.on('error', (err) => {
    console.error('Connection error:', err);
  });

  client.on('close', () => {
    console.log('Connection closed');
  });
}

// Initialisering när komponenten monteras
onMount(() => {
  loadLeaderboard();
  createJoystick();
  setUpMQTT();
});
</script>

<img id="racing" src="racing.png" />
<div class="slider" on:touchstart={moveThumb} on:touchmove={moveThumb} on:click={moveThumb}>
    <span class="value top">255</span>
    <span class="value middle">120</span>
    <div class="thumb" bind:this={thumb}></div>
</div>
<div id="leaderboard">
    {#each players as player, index}
        <div>{player}</div>
    {/each}
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
    #leaderboard{
        position: absolute;
        left: 50%;
    }
    :global(.joystick-container) {
        z-index: 1 !important;
    }
</style>
