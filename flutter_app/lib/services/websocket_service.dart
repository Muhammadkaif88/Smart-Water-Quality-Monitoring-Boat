import 'dart:async';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:web_socket_channel/web_socket_channel.dart';
import 'package:web_socket_channel/status.dart' as status;

class WaterBoatTelemetry {
  final double temperature;
  final double ph;
  final double turbidity;
  final String turbidityStatus;
  final double tds;
  final bool fan;
  final int speed;

  WaterBoatTelemetry({
    this.temperature = 0.0,
    this.ph = 7.0,
    this.turbidity = 0.0,
    this.turbidityStatus = 'Clear',
    this.tds = 0.0,
    this.fan = false,
    this.speed = 200,
  });

  factory WaterBoatTelemetry.fromJson(Map<String, dynamic> json) {
    return WaterBoatTelemetry(
      temperature: (json['temp'] as num?)?.toDouble() ?? 0.0,
      ph: (json['ph'] as num?)?.toDouble() ?? 7.0,
      turbidity: (json['turb'] as num?)?.toDouble() ?? 0.0,
      turbidityStatus: json['turbStatus'] ?? 'Clear',
      tds: (json['tds'] as num?)?.toDouble() ?? 0.0,
      fan: json['fan'] == true || json['fan'] == 'true',
      speed: (json['speed'] as num?)?.toInt() ?? 200,
    );
  }
}

class WebSocketService extends ChangeNotifier {
  static final WebSocketService _instance = WebSocketService._internal();
  factory WebSocketService() => _instance;
  WebSocketService._internal();

  WebSocketChannel? _channel;
  bool _isConnected = false;
  String _hostIp = "192.168.4.1";
  int _port = 81;

  WaterBoatTelemetry _telemetry = WaterBoatTelemetry();
  Timer? _reconnectTimer;
  bool _shouldReconnect = true;

  bool get isConnected => _isConnected;
  String get hostIp => _hostIp;
  WaterBoatTelemetry get telemetry => _telemetry;

  void setHost(String ip, {int port = 81}) {
    _hostIp = ip;
    _port = port;
    disconnect();
    connect();
  }

  void connect() {
    if (_isConnected) return;
    _shouldReconnect = true;

    try {
      final uri = Uri.parse('ws://$_hostIp:$_port/');
      _channel = WebSocketChannel.connect(uri);

      _channel!.stream.listen(
        (data) {
          if (!_isConnected) {
            _isConnected = true;
          }
          _handleMessage(data.toString());
        },
        onDone: () {
          _handleDisconnect();
        },
        onError: (error) {
          debugPrint("WebSocket error: $error");
          _handleDisconnect();
        },
        cancelOnError: true,
      );
    } catch (e) {
      debugPrint("Connection failed: $e");
      _handleDisconnect();
    }
  }

  void _handleMessage(String message) {
    try {
      final Map<String, dynamic> data = jsonDecode(message);
      _telemetry = WaterBoatTelemetry.fromJson(data);
      notifyListeners();
    } catch (e) {
      debugPrint("Error parsing message: $e");
    }
  }

  void _handleDisconnect() {
    _isConnected = false;
    _channel = null;
    notifyListeners();

    if (_shouldReconnect) {
      _reconnectTimer?.cancel();
      _reconnectTimer = Timer(const Duration(seconds: 2), () {
        if (!_isConnected && _shouldReconnect) {
          connect();
        }
      });
    }
  }

  void sendCommand(String cmd) {
    if (_isConnected && _channel != null) {
      try {
        _channel!.sink.add(cmd);
      } catch (e) {
        debugPrint("Error sending command: $e");
      }
    }
  }

  void moveForward() => sendCommand("CMD_FORWARD");
  void moveBackward() => sendCommand("CMD_BACKWARD");
  void turnLeft() => sendCommand("CMD_LEFT");
  void turnRight() => sendCommand("CMD_RIGHT");
  void stop() => sendCommand("CMD_STOP");

  void setSpeed(int speed) {
    int s = speed.clamp(0, 255);
    sendCommand("SPEED:$s");
  }

  void setFan(bool enable) {
    sendCommand(enable ? "FAN_ON" : "FAN_OFF");
  }

  void calibratePh() {
    sendCommand("CAL_PH");
  }

  void disconnect() {
    _shouldReconnect = false;
    _reconnectTimer?.cancel();
    _channel?.sink.close(status.goingAway);
    _channel = null;
    _isConnected = false;
    notifyListeners();
  }

  @override
  void dispose() {
    disconnect();
    super.dispose();
  }
}
