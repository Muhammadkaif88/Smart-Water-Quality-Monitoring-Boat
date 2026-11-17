import 'package:flutter/material.dart';
import '../services/websocket_service.dart';

class DPadController extends StatefulWidget {
  final WebSocketService wsService;

  const DPadController({super.key, required this.wsService});

  @override
  State<DPadController> createState() => _DPadControllerState();
}

class _DPadControllerState extends State<DPadController> {
  String _activeCommand = "READY";

  void _onPress(String label, VoidCallback action) {
    setState(() => _activeCommand = label);
    action();
  }

  void _onRelease() {
    setState(() => _activeCommand = "STOPPED");
    widget.wsService.stop();
  }

  Widget _buildButton({
    required IconData icon,
    required String label,
    required VoidCallback onPress,
    bool isStop = false,
  }) {
    final bool isCurrentActive = _activeCommand == label;

    return GestureDetector(
      onTapDown: (_) => _onPress(label, onPress),
      onTapUp: (_) => _onRelease(),
      onTapCancel: () => _onRelease(),
      child: AnimatedContainer(
        duration: const Duration(milliseconds: 100),
        decoration: BoxDecoration(
          borderRadius: BorderRadius.circular(16),
          gradient: isCurrentActive
              ? const LinearGradient(
                  colors: [Color(0xFF00D2FF), Color(0xFF3A7BD5)],
                  begin: Alignment.topLeft,
                  end: Alignment.bottomRight,
                )
              : LinearGradient(
                  colors: isStop
                      ? [const Color(0xFF3A1A22), const Color(0xFF261016)]
                      : [const Color(0xFF1E2B45), const Color(0xFF141E33)],
                  begin: Alignment.topLeft,
                  end: Alignment.bottomRight,
                ),
          border: Border.all(
            color: isStop
                ? const Color(0xFFFF5E7E).withOpacity(0.4)
                : isCurrentActive
                    ? const Color(0xFF00D2FF)
                    : Colors.white.withOpacity(0.08),
            width: 1.5,
          ),
          boxShadow: isCurrentActive
              ? [
                  BoxShadow(
                    color: const Color(0xFF00D2FF).withOpacity(0.45),
                    blurRadius: 15,
                    spreadRadius: 1,
                  ),
                ]
              : [
                  BoxShadow(
                    color: Colors.black.withOpacity(0.35),
                    blurRadius: 8,
                    offset: const Offset(0, 4),
                  ),
                ],
        ),
        child: Center(
          child: isStop
              ? const Text(
                  "STOP",
                  style: TextStyle(
                    color: Color(0xFFFF5E7E),
                    fontWeight: FontWeight.w800,
                    fontSize: 13,
                    letterSpacing: 1,
                  ),
                )
              : Icon(
                  icon,
                  size: 28,
                  color: isCurrentActive ? Colors.black : Colors.white,
                ),
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: const Color(0xFF131D31).withOpacity(0.85),
        borderRadius: BorderRadius.circular(22),
        border: Border.all(color: Colors.white.withOpacity(0.08)),
        boxShadow: [
          BoxShadow(
            color: Colors.black.withOpacity(0.35),
            blurRadius: 16,
            offset: const Offset(0, 6),
          ),
        ],
      ),
      child: Column(
        children: [
          // Header
          Row(
            mainAxisAlignment: MainAxisAlignment.spaceBetween,
            children: [
              const Row(
                children: [
                  Text("🎮 ", style: TextStyle(fontSize: 16)),
                  Text(
                    "Wireless Navigation",
                    style: TextStyle(
                      fontWeight: FontWeight.w700,
                      fontSize: 14,
                      color: Colors.white,
                    ),
                  ),
                ],
              ),
              Container(
                padding: const EdgeInsets.symmetric(horizontal: 10, vertical: 4),
                decoration: BoxDecoration(
                  color: const Color(0xFF00D2FF).withOpacity(0.12),
                  borderRadius: BorderRadius.circular(12),
                  border: Border.all(
                    color: const Color(0xFF00D2FF).withOpacity(0.3),
                  ),
                ),
                child: Text(
                  _activeCommand,
                  style: const TextStyle(
                    fontSize: 11,
                    fontWeight: FontWeight.w700,
                    color: Color(0xFF00D2FF),
                  ),
                ),
              ),
            ],
          ),
          const SizedBox(height: 16),

          // 3x3 D-Pad Grid
          SizedBox(
            width: 220,
            height: 220,
            child: GridView.count(
              crossAxisCount: 3,
              mainAxisSpacing: 10,
              crossAxisSpacing: 10,
              physics: const NeverScrollableScrollPhysics(),
              children: [
                const SizedBox.shrink(), // (0,0)
                _buildButton(
                  icon: Icons.arrow_upward_rounded,
                  label: "FORWARD",
                  onPress: widget.wsService.moveForward,
                ), // (0,1)
                const SizedBox.shrink(), // (0,2)

                _buildButton(
                  icon: Icons.arrow_back_rounded,
                  label: "LEFT",
                  onPress: widget.wsService.turnLeft,
                ), // (1,0)
                _buildButton(
                  icon: Icons.stop_rounded,
                  label: "STOP",
                  isStop: true,
                  onPress: widget.wsService.stop,
                ), // (1,1)
                _buildButton(
                  icon: Icons.arrow_forward_rounded,
                  label: "RIGHT",
                  onPress: widget.wsService.turnRight,
                ), // (1,2)

                const SizedBox.shrink(), // (2,0)
                _buildButton(
                  icon: Icons.arrow_downward_rounded,
                  label: "BACKWARD",
                  onPress: widget.wsService.moveBackward,
                ), // (2,1)
                const SizedBox.shrink(), // (2,2)
              ],
            ),
          ),
          const SizedBox(height: 10),
          Text(
            "Hold button to navigate • Release to stop",
            style: TextStyle(
              fontSize: 11,
              color: Colors.white.withOpacity(0.4),
            ),
          ),
        ],
      ),
    );
  }
}
