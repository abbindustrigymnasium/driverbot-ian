# Planering 


## Introduktion
Mitt mål med detta projekt är att skapa ett racingspel för min bil genom en webbapplikation, där jag ska använda en ultraljudssensor för att känna igen när bilen når mållinjen och möjligen ett system för hinder och power-ups längs banan. I webbapplikationen kommer spelaren att kunna välja vilken bana hen vill spela, och det kommer finnas instruktioner för var mållinjen och de olika hindren/power-ups ska placeras runt banan. Mer om hur systemet kommer att fungera längre fram.

## Fungernade Mikrocontroller 

Först och främst måste jag få arduino IDE och alla delar i microcontrollern att fungera. Eftersom jag tidigare har arbetat med NodeMCU har jag redan alla drivrutiner och bibliotek som jag behöver för projektet. Jag har också erfarenhet av esp8266 och hur man använder en mikrocontroller och drivershield, så jag tar bara måndagens lektioner åt det.

## Skicka information
När jag är säker att alla komponenter fungerar blir nästa steg är att kunna skicka information till bilen från en webbsida. Jag börjar med att skapa ett Node.js-projekt för att kunna hosta min hemsida och ta emot information från ESPn. Personligen skulle jag ha använt websockets eftersom jag är mer bekant med det, men skolan rekommenderar att använda MQTT, så jag kommer att följa rekommendationerna. För att kommunicera med MQTT genom Node.js kommer jag behöva installera Mosca. Därefter skapar jag en demoapp för att testa om jag kan kommunicera med ESPn. Nedan är den ungefärliga strukturen för appen.


**main.js**

```js
// Starta mosca servern
server = Mosca.server()

// Importera pub och sub
import pub.js
import sub.js

server.on("ready") -> 
	 // Logga för debugging
server.on("clientConnected") -> 
	// Logga för debugging
server.on("publlished") -> 
	 // Logga för debugging
```
**pub.js**
```js
// Koppla till mosca servern
client = mqtt.connect()

// Prova att skicka ett meddelande
client.on("connect") -> 
	client.publish("Hello world") // Loopa hela tiden
```
**sub.js**
```js
// Koppla till mosca servern
client = mqtt.connect()

// Subscriba så fort man är connectad till clienten
client.on("connect") -> 
	client.subscribe() 

client.on("messages") -> 
	// Logga medelandet
```

**ESP.ino**
```arduino
client = PubSubClient()

void callback() -> 
	// Logga när man får ett meddelande

void setup() ->
	// Koppla till wifi
	client.setSever() // Starta clienten på min server
	client.setCallback(callback) // Sätt callbacken

void loop() ->
	om inte connectad ->
		client.connect()
		client.subscribe("min topic")
```

När jag har säkerställt att MQTT fungerar kan jag börja utveckla funktionerna för min bil. I bilen finns en motor och en servo som behöver styras, och jag vill kunna kontrollera dem från min hemsida. Jag börjar med att skapa ett objekt för att hålla datan från bilen. För att undvika långsamhet kommer ESP:n endast att hantera rörelsen av servot och motorn. Beslut om hur mycket de ska flyttas eller vilken hastighet de ska ha hanteras på hemsidan, eftersom den förmodligen är snabbare än ESP:n. Om vi ska framåt skickas till exempel `{1: [160, 1]}`, där 1 representerar motorn och 0 servot, 160 är hastigheten, och 1 är riktningen (framåt eller bakåt). Jag vill skicka så lite data som möjligt för att säkerställa att paketen når fram så snabbt som möjligt. Jag måste dessutom skicka avståndsdata från sensorn till hemsidan varje loop vilket jag gör med client.publish(). 

## Kontroller och hemsida

När jag nu kan styra komponenterna i min ESP, börjar jag designa hemsidan. Jag ska ha en startsida där man kan välja vilken bana man ska köra, och när man har valt en bana ska en sida dyka upp som visar hur den ska designas. Därefter tas man till en sida där man har styrningen för bilen. Min idé är att ha två pilar för att svänga åt vänster och höger, och två pilar för att köra framåt och bakåt (se skiss nedan). Sidan är tänkt för mobil så att man kan styra bilen som med en kontroller. När jag håller ned en svängningsknapp ska det skickas ett meddelande till bilen varje 25 millisekunder att servon ska svänga 5 grader i den valda riktningen, till exempel {0: [5, 1]}, där 0 representerar vänstersväng och 1 representerar högersväng. För rörelsen hålls en variabel sparad med den nuvarande hastigheten. När man kör framåt kommer alla meddelanden att skickas med den aktuella hastigheten, och när man trycker på knappen för att accelerera eller sakta ner ändras denna variabel beroende på om man vill köra fortare eller långsammare.

Nu till hindren. Jag kommer att använda en ultraljudssensor som pekar mot marken för att mäta hur lång tid det tar för signalen att reflekteras tillbaka. Med den informationen kan jag känna av när ett objekt kommer under sensorn. Jag kan 3D-skriva olika höga bitar så att bilen enkelt kan köra över dem och så att sensorn kan upptäcka skillnaden i tid. Jag kan placera den högsta biten vid mållinjen, så när sensorn känner av att avståndet är cirka 0 mikrosekunder, vet vi att vi har nått målet. Sedan kan jag använda olika höga bitar för olika typer av hinder och power-ups. Ett hinder kan till exempel få bilen att snurra slumpmässigt, medan en power-up kan minska din tid i banan eller göra bilen snabbare. För varje körning av en bana sparas din tid.

## Tidsplan

Nedan är tidsplanen för mitt projekt.

**Måndag 15/4**
Fungerande mikrokontroller

**Tisdag 16/4**
Påbörjad MQTT implementation

**Torsdag 18/4**
Fortsätta med MQTT
mål: kunna skicka data med MQTT till ESPn

**Tisdag 23/4**
Kontrollera alla delar i bilen genom MQTT

**Fredag 26/4**
Påbörja hemsidan
mål: Hemsida och sida för kontrollerna

**Måndag 29/4**
Kunna styra bilen helt från hemsidan

**Tisdag 30/4**
Kunna känna igen målet samt olika typer av power-ups och hinder genom ultraljudssensorn

**Tisdag 7/5**
Effekter för varje power up och hinder, börja designa några banor

**Tisdag 14/5**
Designat minst 4 banor

**Tisdag 21/5**
Styla upp hemsidan

**Torsdag, Måndag, Tisdag**
Fixa buggar inför uppkörningen

