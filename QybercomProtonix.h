#pragma once

#include <Arduino.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

namespace Qybercom {
	namespace Protonix {
		class ProtonixDTO;
		class ProtonixDevice;

		class ProtonixURI {
			private:
				String _scheme;
				String _username;
				String _password;
				String _host;
				uint _port;
				String _path;

			public:
				ProtonixURI(String host, uint port);
				ProtonixURI(String host, uint port, String path);

				void Scheme(String scheme);
				String Scheme();

				void Username(String username);
				String Username();

				void Password(String password);
				String Password();

				void Host(String host);
				String Host();

				void Port(uint port);
				uint Port();

				void Path(String path);
				String Path();
		};

		class ProtonixTimer {
			public:
				enum class ProtonixTimerUnit {
					MICROSECONDS,
					MILLISECONDS
				};

				ProtonixTimer();
				ProtonixTimer(unsigned int interval);
				ProtonixTimer(unsigned int interval, ProtonixTimerUnit unit);

				unsigned long Previous();

				void Interval(int interval);
				unsigned int Interval();

				void Unit(ProtonixTimerUnit unit);
				ProtonixTimerUnit Unit();

				bool Pipe();

			private:
				unsigned long _previous;
				unsigned int _interval;
				ProtonixTimerUnit _unit;
		};

		class ProtonixDeviceSensor {
			private:
				String _id;
				String _value;
				bool _active;
				bool _failure;

			public:
				ProtonixDeviceSensor();
				ProtonixDeviceSensor(String id);

				void ID(String id);
				String ID();

				void Value(String value);
				String Value();

				void Active(bool active);
				bool Active();

				void Failure(bool failure);
				bool Failure();
		};

		class ProtonixDeviceStatus {
			private:
				String _summary;

			public:
				ProtonixDeviceStatus();

				void Summary(String summary);
				String Summary();
		};

		class IProtonixDTO {
			public:
				virtual void DTOPopulate(ProtonixDTO* dto);
				virtual void DTOToJSON(JsonDocument& dto);
				virtual String DTOSerialize();
		};

		class IProtonixDTORequest : public IProtonixDTO {
		};

		class IProtonixDTOResponse : public IProtonixDTO {
			public:
				virtual unsigned short DTOResponseStatus();
		};

		class IProtonixDTOEvent : public IProtonixDTO {
		};

		namespace DTO {
			class DTORequestAuthorization : public IProtonixDTORequest {
				private:
					String _id;
					String _passphrase;

				public:
					DTORequestAuthorization();
					DTORequestAuthorization(String id, String passphrase);

					void ID(String id);
					String ID();

					void Passphrase(String passphrase);
					String Passphrase();

					void DTOPopulate(ProtonixDTO* dto);
					void DTOToJSON(JsonDocument& dto);
					String DTOSerialize();
			};

			class DTORequestDeviceStatus : public IProtonixDTORequest {
				private:
					ProtonixDeviceStatus* _status;

				public:
					DTORequestDeviceStatus();
					DTORequestDeviceStatus(ProtonixDeviceStatus* status);

					void Status(ProtonixDeviceStatus* status);
					ProtonixDeviceStatus* Status();

					void DTOPopulate(ProtonixDTO* dto);
					void DTOToJSON(JsonDocument& dto);
					String DTOSerialize();
			};

			class DTOResponseAuthorization : public IProtonixDTOResponse {
				private:
					unsigned short _status;

				public:
					void Status(unsigned short status);
					unsigned short Status();

					void DTOPopulate(ProtonixDTO* dto);
					void DTOToJSON(JsonDocument& dto);
					String DTOSerialize();
					unsigned short DTOResponseStatus();
			};

			class DTOEventCommand : public IProtonixDTOEvent {
				private:
					String _name;

				public:
					void Name(String name);
					String Name();

					void DTOPopulate(ProtonixDTO* dto);
					void DTOToJSON(JsonDocument& dto);
					String DTOSerialize();
			};
		}

		class ProtonixDTO {
			private:
				String _url;
				String _response;
				String _event;
				IProtonixDTO* _dto;
				JsonObject _data;
				bool _debug;
				StaticJsonDocument<1024> _buffer;
				bool _bufferInit;
				String _bufferOutput;

			public:
				ProtonixDTO();

				void URL(String url);
				String URL();

				void Response(String url);
				String Response();

				void Event(String url);
				String Event();

				void DTO(IProtonixDTO* dto);
				IProtonixDTO* DTO();
				void Data(JsonObject data);
				JsonObject Data();

				bool IsURL();
				bool IsResponse();
				bool IsEvent();

				String Serialize();
				bool Deserialize(String raw);

				void Debug(bool debug);
				bool Debug();
		};

		class INetwork {
			public:
				virtual bool Connect();
				virtual bool Connected();
				virtual String AddressMAC();
				virtual String AddressIP();

				// https://stackoverflow.com/a/12772708/2097055
				// https://stackoverflow.com/a/1755042/2097055
				static void ParseMAC (String mac, uint8_t out[6]) {
					char buffer[18];
					mac.toCharArray(buffer, 18);

					sscanf(buffer, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &out[0], &out[1], &out[2], &out[3], &out[4], &out[5]);
				}
		};

		class IProtocol {
			public:
				virtual void Init(ProtonixDevice* device);
				virtual bool Connect(ProtonixURI* uri);
				virtual bool Connected();
				virtual void Pipe();
				virtual void Send(String data);
		};

		namespace Networks {
			class NWiFi : public INetwork {
				private:
					String _ssid;
					String _password;
					String _mac;
					String _hostname;
				
				public:
					NWiFi(String ssid, String password, String mac, String hostname);

					bool Connect();

					bool Connected();

					String AddressMAC();

					String AddressIP();
			};
		}

		namespace Protocols {
			class PWiFiTCP : public IProtocol {
				private:
					WiFiClient _client;
					ProtonixDevice* _device;
					unsigned char _buffer[1024];
					unsigned char* _bufferPTR;

				public:
					void Init(ProtonixDevice* device);
					bool Connect(ProtonixURI* uri);
					bool Connected();
					void Pipe();
					void Send(String data);
			};

			class PWebSocket : public IProtocol {
				private:
					websockets::WebsocketsClient _client;
					ProtonixDevice* _device;

				public:
					void Init(ProtonixDevice* device);
					bool Connect(ProtonixURI* uri);
					bool Connected();
					void Pipe();
					void Send(String data);
			};
		}

		class IProtonixDevice {
			public:
				virtual unsigned int DeviceTick();
				virtual String DeviceID();
				virtual String DevicePassphrase();
				virtual void DeviceOnReady(ProtonixDevice* device);
				virtual void DeviceOnTick(ProtonixDevice* device);
				virtual void DeviceOnNetworkConnect(ProtonixDevice* device);
				virtual void DeviceOnProtocolConnect(ProtonixDevice* device);
				virtual void DeviceOnStreamResponse(ProtonixDevice* device, ProtonixDTO* dto);
				virtual void DeviceOnStreamEvent(ProtonixDevice* device, ProtonixDTO* dto);
				virtual void DeviceOnCommand(ProtonixDevice* device, DTO::DTOEventCommand* command);
				virtual ProtonixDeviceStatus* DeviceStatus();
		};

		// http://tedfelix.com/software/c++-callbacks.html
		class ProtonixDevice {
			public:
				ProtonixDevice(IProtonixDevice* device);

				void Device(IProtonixDevice* device);
				IProtonixDevice* Device();
				ProtonixTimer* Timer();

				void Network(INetwork* network);
				INetwork* Network();

				void Protocol(IProtocol* protocol);
				IProtocol* Protocol();

				void Server(ProtonixURI* uri);
				ProtonixURI* Server();

				void ServerEndpoint(String host, uint port);
				void ServerEndpoint(String host, uint port, String path);

				void Debug(bool debug);
				bool Debug();
				
				void Pipe();

				//void OnStream(String data);
				void OnStream(unsigned char* data);
				void OnStreamResponse(ProtonixDTO* dto);
				void OnStreamEvent(ProtonixDTO* dto);

				void RequestStream(String url, IProtonixDTORequest* request);

			private:
				IProtonixDevice* _device;
				bool _ready;
				ProtonixTimer* _timer;
				INetwork* _network;
				bool _networkConnected1;
				bool _networkConnected2;
				IProtocol* _protocol;
				bool _protocolConnected1;
				bool _protocolConnected2;
				ProtonixURI* _uri;
				bool _debug;
				void _pipe();
		};
	}
}