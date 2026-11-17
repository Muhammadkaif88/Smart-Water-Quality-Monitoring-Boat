import 'dart:math' as math;
import 'package:flutter/material.dart';
import '../services/websocket_service.dart';
import '../widgets/sensor_card.dart';
import '../widgets/dpad_controller.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen>
    with SingleTickerProviderStateMixin {
  final WebSocketService _ws = WebSocketService();
  late AnimationController _fanAnimController;
  final TextEditingController _ipController = TextEditingController();

  @override
  void initState() {
    super.initState();
    _fanAnimController = AnimationController(
      vsync: this,
      duration: const Duration(milliseconds: 800),
    );
    _ipController.text = _ws.hostIp;
    _ws.connect();
    _ws.addListener(_onWsUpdate);
  }

  void _onWsUpdate() {
    if (mounted) {
      if (_ws.telemetry.fan) {
        if (!_fanAnimController.isAnimating) _fanAnimController.repeat();
      } else {
        _fanAnimController.stop();
      }
      setState(() {});
    }
  }

  @override
  void dispose() {
    _ws.removeListener(_onWsUpdate);
    _fanAnimController.dispose();
    _ipController.dispose();
    super.dispose();
  }

  void _showIpDialog() {
    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        backgroundColor: const Color(0xFF131D31),
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(18)),
        title: const Text(
          "Boat IP Settings",
          style: TextStyle(color: Colors.white, fontWeight: FontWeight.w700),
        ),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            const Text(
              "Default Boat AP IP is 192.168.4.1",
              style: TextStyle(color: Colors.white54, fontSize: 13),
            ),
            const SizedBox(height: 12),
            TextField(
              controller: _ipController,
              style: const TextStyle(color: Colors.white),
              decoration: InputDecoration(
                labelText: "IP Address",
                labelStyle: const TextStyle(color: Color(0xFF00D2FF)),
                filled: true,
                fillColor: const Color(0xFF1A2640),
                border: OutlineInputBorder(
                  borderRadius: BorderRadius.circular(12),
                  borderSide: BorderSide.none,
                ),
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: const Text("Cancel", style: TextStyle(color: Colors.white60)),
          ),
          ElevatedButton(
            style: ElevatedButton.styleFrom(
              backgroundColor: const Color(0xFF00D2FF),
              foregroundColor: Colors.black,
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
            ),
            onPressed: () {
              _ws.setHost(_ipController.text.trim());
              Navigator.pop(ctx);
            },
            child: const Text("Save & Connect"),
          ),
        ],
      ),
    );
  }

  void _showPhCalibrateDialog() {
    showDialog(
      context: context,
      builder: (ctx) => AlertDialog(
        backgroundColor: const Color(0xFF131D31),
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(18)),
        title: const Row(
          children: [
            Text("🧪 ", style: TextStyle(fontSize: 20)),
            Text(
              "Calibrate pH Sensor",
              style: TextStyle(color: Colors.white, fontWeight: FontWeight.w700),
            ),
          ],
        ),
        content: const Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(
              "1. Place probe in clean neutral water (pH 7.0).\n2. Tap 'Set 7.0 (Calibrate)' below.\n3. The boat will calibrate this reading as exact pH 7.00!",
              style: TextStyle(color: Colors.white70, fontSize: 13, height: 1.4),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(ctx),
            child: const Text("Cancel", style: TextStyle(color: Colors.white60)),
          ),
          ElevatedButton(
            style: ElevatedButton.styleFrom(
              backgroundColor: const Color(0xFFA78BFA),
              foregroundColor: Colors.black,
              shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
            ),
            onPressed: () {
              _ws.calibratePh();
              Navigator.pop(ctx);
              ScaffoldMessenger.of(context).showSnackBar(
                const SnackBar(
                  content: Text("✅ Sent pH 7.0 Calibration command to Boat!"),
                  backgroundColor: Color(0xFF1E2A44),
                ),
              );
            },
            child: const Text("Set 7.0 (Calibrate)"),
          ),
        ],
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    final telemetry = _ws.telemetry;
    final isConnected = _ws.isConnected;

    // pH status & color
    final ph = telemetry.ph;
    final phStatus = ph < 6.5
        ? 'Acidic 🔴'
        : (ph > 8.5 ? 'Alkaline 🟣' : 'Normal / Safe 🟢');
    final phColor = ph < 6.5
        ? const Color(0xFFFF5E7E)
        : (ph > 8.5 ? const Color(0xFFA78BFA) : const Color(0xFF00F5A0));

    // TDS status & color
    final tds = telemetry.tds;
    final tdsStatus = tds < 300
        ? 'Excellent 🟢'
        : (tds < 600 ? 'Good 🟡' : 'High Dissolved 🔴');
    final tdsColor = tds < 300
        ? const Color(0xFF00F5A0)
        : (tds < 600 ? const Color(0xFFFFD166) : const Color(0xFFFF5E7E));

    // Turbidity color
    final turbColor = telemetry.turbidity <= 100
        ? const Color(0xFF00F5A0)
        : (telemetry.turbidity < 1000
            ? const Color(0xFFFFD166)
            : const Color(0xFFFF5E7E));

    final speedPct = ((telemetry.speed / 255.0) * 100).round();

    return Scaffold(
      backgroundColor: const Color(0xFF090E17),
      body: Container(
        decoration: const BoxDecoration(
          gradient: RadialGradient(
            center: Alignment(0, -0.7),
            radius: 1.2,
            colors: [Color(0xFF17243C), Color(0xFF090E17)],
          ),
        ),
        child: SafeArea(
          child: SingleChildScrollView(
            padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.center,
              children: [
                // Top App Bar
                Container(
                  padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 12),
                  decoration: BoxDecoration(
                    color: const Color(0xFF131D31).withOpacity(0.85),
                    borderRadius: BorderRadius.circular(20),
                    border: Border.all(color: Colors.white.withOpacity(0.08)),
                    boxShadow: [
                      BoxShadow(
                        color: Colors.black.withOpacity(0.3),
                        blurRadius: 16,
                      ),
                    ],
                  ),
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Row(
                        children: [
                          Container(
                            width: 38,
                            height: 38,
                            decoration: BoxDecoration(
                              gradient: const LinearGradient(
                                colors: [Color(0xFF00D2FF), Color(0xFF3A7BD5)],
                              ),
                              borderRadius: BorderRadius.circular(12),
                            ),
                            child: const Center(
                              child: Text("🚤", style: TextStyle(fontSize: 20)),
                            ),
                          ),
                          const SizedBox(width: 10),
                          Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              const Text(
                                "AQUA BOAT",
                                style: TextStyle(
                                  fontWeight: FontWeight.w800,
                                  fontSize: 16,
                                  color: Colors.white,
                                  letterSpacing: 0.5,
                                ),
                              ),
                              Text(
                                "Smart Water Monitor",
                                style: TextStyle(
                                  fontSize: 11,
                                  color: Colors.white.withOpacity(0.5),
                                ),
                              ),
                            ],
                          ),
                        ],
                      ),
                      Row(
                        children: [
                          // Status Badge
                          Container(
                            padding: const EdgeInsets.symmetric(
                                horizontal: 10, vertical: 5),
                            decoration: BoxDecoration(
                              color: isConnected
                                  ? const Color(0xFF00F5A0).withOpacity(0.12)
                                  : const Color(0xFFFF5E7E).withOpacity(0.12),
                              borderRadius: BorderRadius.circular(16),
                              border: Border.all(
                                color: isConnected
                                    ? const Color(0xFF00F5A0).withOpacity(0.3)
                                    : const Color(0xFFFF5E7E).withOpacity(0.3),
                              ),
                            ),
                            child: Row(
                              children: [
                                Container(
                                  width: 7,
                                  height: 7,
                                  decoration: BoxDecoration(
                                    color: isConnected
                                        ? const Color(0xFF00F5A0)
                                        : const Color(0xFFFF5E7E),
                                    shape: BoxShape.circle,
                                  ),
                                ),
                                const SizedBox(width: 6),
                                Text(
                                  isConnected ? "Connected" : "Disconnected",
                                  style: TextStyle(
                                    fontSize: 11,
                                    fontWeight: FontWeight.w700,
                                    color: isConnected
                                        ? const Color(0xFF00F5A0)
                                        : const Color(0xFFFF5E7E),
                                  ),
                                ),
                              ],
                            ),
                          ),
                          const SizedBox(width: 6),
                          IconButton(
                            icon: const Icon(Icons.settings_rounded,
                                size: 20, color: Colors.white70),
                            onPressed: _showIpDialog,
                          ),
                        ],
                      ),
                    ],
                  ),
                ),

                const SizedBox(height: 16),

                // 2x2 Sensor Grid
                GridView.count(
                  crossAxisCount: 2,
                  shrinkWrap: true,
                  physics: const NeverScrollableScrollPhysics(),
                  crossAxisSpacing: 12,
                  mainAxisSpacing: 12,
                  childAspectRatio: 1.20,
                  children: [
                    GestureDetector(
                      onTap: isConnected ? () => _ws.sendCommand("TOGGLE_PH") : null,
                      child: SensorCard(
                        title: "pH Level",
                        value: isConnected ? ph.toStringAsFixed(2) : "--",
                        unit: "pH",
                        status: isConnected ? phStatus : "Offline",
                        icon: "🧪",
                        accentColor: phColor,
                      ),
                    ),
                    SensorCard(
                      title: "Water Temp",
                      value: isConnected
                          ? telemetry.temperature.toStringAsFixed(1)
                          : "--",
                      unit: "°C",
                      status: isConnected ? "DHT11 Live" : "Offline",
                      icon: "🌡️",
                      accentColor: const Color(0xFFFF8C42),
                    ),
                    SensorCard(
                      title: "Turbidity",
                      value: isConnected
                          ? telemetry.turbidity.toStringAsFixed(0)
                          : "--",
                      unit: "NTU",
                      status: isConnected ? telemetry.turbidityStatus : "Offline",
                      icon: "🌫️",
                      accentColor: turbColor,
                    ),
                    SensorCard(
                      title: "TDS Level",
                      value: isConnected
                          ? telemetry.tds.toStringAsFixed(0)
                          : "--",
                      unit: "ppm",
                      status: isConnected ? tdsStatus : "Offline",
                      icon: "💧",
                      accentColor: tdsColor,
                    ),
                  ],
                ),

                const SizedBox(height: 14),

                // Fan Relay Control Card
                Container(
                  padding: const EdgeInsets.symmetric(horizontal: 18, vertical: 14),
                  decoration: BoxDecoration(
                    color: const Color(0xFF131D31).withOpacity(0.85),
                    borderRadius: BorderRadius.circular(20),
                    border: Border.all(color: Colors.white.withOpacity(0.08)),
                    boxShadow: [
                      BoxShadow(
                        color: Colors.black.withOpacity(0.25),
                        blurRadius: 12,
                        offset: const Offset(0, 4),
                      ),
                    ],
                  ),
                  child: Row(
                    mainAxisAlignment: MainAxisAlignment.spaceBetween,
                    children: [
                      Row(
                        children: [
                          AnimatedBuilder(
                            animation: _fanAnimController,
                            builder: (context, child) {
                              return Transform.rotate(
                                angle: _fanAnimController.value * 2 * math.pi,
                                child: child,
                              );
                            },
                            child: Container(
                              width: 42,
                              height: 42,
                              decoration: BoxDecoration(
                                color: telemetry.fan
                                    ? const Color(0xFF00D2FF).withOpacity(0.2)
                                    : Colors.white.withOpacity(0.05),
                                borderRadius: BorderRadius.circular(12),
                                border: Border.all(
                                  color: telemetry.fan
                                      ? const Color(0xFF00D2FF)
                                      : Colors.transparent,
                                ),
                              ),
                              child: const Center(
                                child: Text("🪭", style: TextStyle(fontSize: 22)),
                              ),
                            ),
                          ),
                          const SizedBox(width: 14),
                          Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              const Text(
                                "Cleaning Motor",
                                style: TextStyle(
                                  fontWeight: FontWeight.w700,
                                  fontSize: 14,
                                  color: Colors.white,
                                ),
                              ),
                              Text(
                                "Relay (GPIO 13)",
                                style: TextStyle(
                                  fontSize: 11,
                                  color: Colors.white.withOpacity(0.5),
                                ),
                              ),
                            ],
                          ),
                        ],
                      ),
                      Switch.adaptive(
                        value: telemetry.fan,
                        activeColor: const Color(0xFF00D2FF),
                        activeTrackColor:
                            const Color(0xFF00D2FF).withOpacity(0.4),
                        onChanged: (val) => _ws.setFan(val),
                      ),
                    ],
                  ),
                ),

                const SizedBox(height: 14),

                // Speed Slider (ENA / ENB PWM)
                Container(
                  padding: const EdgeInsets.symmetric(horizontal: 18, vertical: 14),
                  decoration: BoxDecoration(
                    color: const Color(0xFF131D31).withOpacity(0.85),
                    borderRadius: BorderRadius.circular(20),
                    border: Border.all(color: Colors.white.withOpacity(0.08)),
                    boxShadow: [
                      BoxShadow(
                        color: Colors.black.withOpacity(0.25),
                        blurRadius: 12,
                        offset: const Offset(0, 4),
                      ),
                    ],
                  ),
                  child: Column(
                    children: [
                      Row(
                        mainAxisAlignment: MainAxisAlignment.spaceBetween,
                        children: [
                          const Text(
                            "⚡ Boat Speed Control",
                            style: TextStyle(
                              fontWeight: FontWeight.w700,
                              fontSize: 14,
                              color: Colors.white,
                            ),
                          ),
                          Text(
                            "$speedPct%",
                            style: const TextStyle(
                              fontWeight: FontWeight.w800,
                              fontSize: 16,
                              color: Color(0xFF00D2FF),
                            ),
                          ),
                        ],
                      ),
                      const SizedBox(height: 6),
                      SliderTheme(
                        data: SliderTheme.of(context).copyWith(
                          trackHeight: 6,
                          activeTrackColor: const Color(0xFF00D2FF),
                          inactiveTrackColor: const Color(0xFF1E2B45),
                          thumbColor: const Color(0xFF00D2FF),
                          overlayColor: const Color(0xFF00D2FF).withOpacity(0.2),
                          thumbShape: const RoundSliderThumbShape(
                              enabledThumbRadius: 10),
                        ),
                        child: Slider(
                          min: 0,
                          max: 255,
                          value: telemetry.speed.toDouble().clamp(0, 255),
                          onChanged: (val) => _ws.setSpeed(val.round()),
                        ),
                      ),
                      Row(
                        children: [
                          Expanded(
                            child: OutlinedButton(
                              style: OutlinedButton.styleFrom(
                                side: BorderSide(
                                    color: Colors.white.withOpacity(0.1)),
                                shape: RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(10)),
                              ),
                              onPressed: () => _ws.setSpeed(120),
                              child: const Text("Slow (45%)",
                                  style: TextStyle(
                                      fontSize: 11, color: Colors.white70)),
                            ),
                          ),
                          const SizedBox(width: 8),
                          Expanded(
                            child: OutlinedButton(
                              style: OutlinedButton.styleFrom(
                                side: BorderSide(
                                    color: Colors.white.withOpacity(0.1)),
                                shape: RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(10)),
                              ),
                              onPressed: () => _ws.setSpeed(190),
                              child: const Text("Cruise (75%)",
                                  style: TextStyle(
                                      fontSize: 11, color: Colors.white70)),
                            ),
                          ),
                          const SizedBox(width: 8),
                          Expanded(
                            child: OutlinedButton(
                              style: OutlinedButton.styleFrom(
                                side: BorderSide(
                                    color: Colors.white.withOpacity(0.1)),
                                shape: RoundedRectangleBorder(
                                    borderRadius: BorderRadius.circular(10)),
                              ),
                              onPressed: () => _ws.setSpeed(255),
                              child: const Text("Turbo (100%)",
                                  style: TextStyle(
                                      fontSize: 11, color: Colors.white70)),
                            ),
                          ),
                        ],
                      ),
                    ],
                  ),
                ),

                const SizedBox(height: 14),

                // D-Pad Boat Controller
                DPadController(wsService: _ws),
              ],
            ),
          ),
        ),
      ),
    );
  }
}
