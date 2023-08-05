#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>

#include "QybercomProtonix.h"

using namespace Qybercom::Protonix;


ProtonixURI::ProtonixURI (String host, uint port) {
	this->Host(host);
	this->Port(port);
}

ProtonixURI::ProtonixURI (String host, uint port, String path) {
	this->Host(host);
	this->Port(port);
	this->Path(path);
}

void ProtonixURI::Scheme (String scheme) {
	this->_scheme = scheme;
}

String ProtonixURI::Scheme () {
	return this->_scheme;
}

void ProtonixURI::Username (String username) {
	this->_username = username;
}

String ProtonixURI::Username () {
	return this->_username;
}

void ProtonixURI::Password (String password) {
	this->_password = password;
}

String ProtonixURI::Password () {
	return this->_password;
}

void ProtonixURI::Host (String host) {
	this->_host = host;
}

String ProtonixURI::Host () {
	return this->_host;
}

void ProtonixURI::Port (uint port) {
	this->_port = port;
}

uint ProtonixURI::Port () {
	return this->_port;
}

void ProtonixURI::Path (String path) {
	this->_path = path;
}

String ProtonixURI::Path () {
	return this->_path;
}




ProtonixTimer::ProtonixTimer () {
	this->_previous = 0;

	this->Interval(0);
	this->Unit(ProtonixTimer::ProtonixTimerUnit::MILLISECONDS);
}

ProtonixTimer::ProtonixTimer (unsigned int interval) {
	this->_previous = 0;

	this->Interval(interval);
	this->Unit(ProtonixTimer::ProtonixTimerUnit::MILLISECONDS);
}

ProtonixTimer::ProtonixTimer (unsigned int interval, ProtonixTimer::ProtonixTimerUnit unit) {
	this->_previous = 0;

	this->Interval(interval);
	this->Unit(unit);
}

unsigned long ProtonixTimer::Previous () {
	return this->_previous;
}

void ProtonixTimer::Interval (int interval) {
	this->_interval = interval;
}

unsigned int ProtonixTimer::Interval () {
	return this->_interval;
}

void ProtonixTimer::Unit (ProtonixTimer::ProtonixTimerUnit unit) {
	this->_unit = unit;
}

ProtonixTimer::ProtonixTimerUnit ProtonixTimer::Unit () {
	return this->_unit;
}

bool ProtonixTimer::Pipe () {
	unsigned long current = 0;

	if (this->_unit == ProtonixTimer::ProtonixTimerUnit::MILLISECONDS)
		current = millis();

	if (this->_unit == ProtonixTimer::ProtonixTimerUnit::MICROSECONDS)
		current = micros();

	long diff = this->_previous - current;

	return diff < 0 || diff >= this->_interval;
}




ProtonixProtocolDTO::ProtonixProtocolDTO () {
	//this->_dto = DynamicJsonDocument<512>;
}

void ProtonixProtocolDTO::URL (String url) {
	this->_url = url;
}
String ProtonixProtocolDTO::URL () {
	return this->_url;
}

void ProtonixProtocolDTO::Response (String url) {
	this->_response = url;
}
String ProtonixProtocolDTO::Response () {
	return this->_response;
}

void ProtonixProtocolDTO::Event (String url) {
	this->_event = url;
}
String ProtonixProtocolDTO::Event () {
	return this->_event;
}

void ProtonixProtocolDTO::Data (JsonObject data) {
	this->_data = data;
}

JsonObject ProtonixProtocolDTO::Data () {
	return this->_data;
}

bool ProtonixProtocolDTO::IsURL () {
	return this->_url.length() != 0;
}

bool ProtonixProtocolDTO::IsResponse () {
	return this->_response.length() != 0;
}

bool ProtonixProtocolDTO::IsEvent () {
	return this->_event.length() != 0;
}

String ProtonixProtocolDTO::Serialize () {
	String out;
	serializeJson(this->_dto, out);

	return out;
}

bool ProtonixProtocolDTO::Deserialize (String raw) {
	deserializeJson(this->_dto, raw);
	JsonObject dto = this->_dto.as<JsonObject>();

	if (dto.containsKey("url")) {
		const char* url = dto["url"];
		this->_url = (String)url;
	}

	if (dto.containsKey("response")) {
		const char* url = dto["response"];
		this->_response = (String)url;
	}

	if (dto.containsKey("event")) {
		const char* url = dto["event"];
		this->_event = (String)url;
	}

	if (dto.containsKey("data"))
		this->_data = dto["data"];

	return true;
}





Networks::NWiFi::NWiFi (String ssid, String password, String mac, String hostname) {
	this->_ssid = ssid;
	this->_password = password;
	this->_mac = mac;
	this->_hostname = hostname;
}

bool Networks::NWiFi::Connect () {
	uint8_t mac[6] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	INetwork::ParseMAC(this->_mac, mac);

	// https://randomnerdtutorials.com/esp32-set-custom-hostname-arduino/#comment-741757
	WiFi.setHostname(this->_hostname.c_str());
	WiFi.mode(WIFI_STA);
	esp_wifi_set_mac(WIFI_IF_STA, &mac[0]);
	WiFi.begin(this->_ssid, this->_password);

	return true;
}

bool Networks::NWiFi::Connected () {
	return WiFi.status() == WL_CONNECTED;
}

String Networks::NWiFi::AddressMAC () {
	return this->_mac;
}

String Networks::NWiFi::AddressIP () {
	return String(WiFi.localIP());
}





Protocols::PWebSocket::PWebSocket () {
	this->_client.onMessage([&](websockets::WebsocketsMessage message) {
		Serial.println("[PWebSocket::onMessage] " + message.data());

		ProtonixProtocolDTO* dto = new ProtonixProtocolDTO();
		dto->Deserialize(message.data());

		if (dto->IsURL())
			Serial.println("[PWebSocket::onMessage URL] " + dto->URL());

		if (dto->IsResponse())
			Serial.println("[PWebSocket::onMessage Response] " + dto->Response());

		if (dto->IsEvent())
			Serial.println("[PWebSocket::onMessage Event] " + dto->Event());
	});
}

bool Protocols::PWebSocket::Connect (ProtonixURI* uri) {
	return this->_client.connect(
		uri->Host(),
		uri->Port(),
		uri->Path()
	);
}

bool Protocols::PWebSocket::Connected () {
	return this->_client.available();
}

void Protocols::PWebSocket::Pipe() {
	this->_client.poll();
}




ProtonixDevice::ProtonixDevice (IProtonixDevice* device) {
	this->_ready = false;
	this->_networkConnected1 = false;
	this->_networkConnected2 = false;
	this->_protocolConnected1 = false;
	this->_protocolConnected2 = false;

	this->Device(device);
	this->_timer = new ProtonixTimer(this->_device->DeviceTick());
}

void ProtonixDevice::Device (IProtonixDevice* device) {
	this->_device = device;
}

IProtonixDevice* ProtonixDevice::Device () {
	return this->_device;
}

ProtonixTimer* ProtonixDevice::Timer () {
	return this->_timer;
}

void ProtonixDevice::Network (INetwork* network) {
	this->_network = network;
}

INetwork* ProtonixDevice::Network () {
	return this->_network;
}

void ProtonixDevice::Protocol (IProtocol* protocol) {
	this->_protocol = protocol;
}

IProtocol* ProtonixDevice::Protocol () {
	return this->_protocol;
}

void ProtonixDevice::Server (ProtonixURI* uri) {
	this->_uri = uri;
}

ProtonixURI* ProtonixDevice::Server () {
	return this->_uri;
}

void ProtonixDevice::ServerEndpoint (String host, uint port) {
	this->Server(new ProtonixURI(host, port));
}

void ProtonixDevice::ServerEndpoint (String host, uint port, String path) {
	this->Server(new ProtonixURI(host, port, path));
}

void ProtonixDevice::_pipe () {
	if (!this->_networkConnected1 || !this->_networkConnected2) {
		if (!this->_networkConnected1) {
			Serial.println("[network:connect]");

			this->_network->Connect();
			this->_networkConnected1 = true;
		}

		if (!this->_network->Connected()) return;

		Serial.println("[network:connected]");
		this->_networkConnected2 = true;
		this->_device->DeviceOnNetworkConnect(this);
	}

	if (!this->_protocolConnected1 || !this->_protocolConnected2) {
		if (!this->_protocolConnected1) {
			Serial.println("[protocol:connect]");

			this->_protocol->Connect(this->_uri);
			this->_protocolConnected1 = true;
		}

		if (!this->_protocol->Connected()) return;

		Serial.println("[protocol:connected]");
		this->_protocolConnected2 = true;
		this->_device->DeviceOnProtocolConnect(this);
	}

	this->_protocol->Pipe();
}

void ProtonixDevice::Pipe () {
	if (!this->_ready) {
		this->_ready = true;
		this->_device->DeviceOnReady(this);
	}

	if (this->_timer->Pipe())
		this->_pipe();
}