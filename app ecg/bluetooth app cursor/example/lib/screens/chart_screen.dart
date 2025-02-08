import 'dart:async';
import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:fl_chart/fl_chart.dart';

class ChartScreen extends StatefulWidget {
  final BluetoothDevice device;

  const ChartScreen({Key? key, required this.device}) : super(key: key);

  @override
  _ChartScreenState createState() => _ChartScreenState();
}

class _ChartScreenState extends State<ChartScreen> {
  List<FlSpot> _data = [];
  StreamSubscription<List<int>>? _dataSubscription;

  @override
  void initState() {
    super.initState();
    _connectToDevice();
  }

  void _connectToDevice() async {
    await widget.device.connect();
    List<BluetoothService> services = await widget.device.discoverServices();
    BluetoothCharacteristic? targetCharacteristic;

    for (var service in services) {
      for (var characteristic in service.characteristics) {
        if (characteristic.properties.notify) {
          targetCharacteristic = characteristic;
          break;
        }
      }
      if (targetCharacteristic != null) break;
    }

    if (targetCharacteristic != null) {
      await targetCharacteristic.setNotifyValue(true);
      _dataSubscription = targetCharacteristic.lastValueStream.listen((value) {
        if (value.isNotEmpty) {
          setState(() {
            _data.add(FlSpot(_data.length.toDouble(), value[0].toDouble()));
            if (_data.length > 50) _data.removeAt(0);
          });
        }
      });
    }
  }

  @override
  void dispose() {
    _dataSubscription?.cancel();
    widget.device.disconnect();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text('Bluetooth Data Chart')),
      body: _data.isEmpty
          ? Center(child: CircularProgressIndicator())
          : Padding(
              padding: const EdgeInsets.all(16.0),
              child: LineChart(
                LineChartData(
                  lineBarsData: [
                    LineChartBarData(
                      spots: _data,
                      isCurved: true,
                      color: Colors.blue,
                      barWidth: 3,
                      isStrokeCapRound: true,
                      dotData: FlDotData(show: false),
                    ),
                  ],
                  titlesData: FlTitlesData(show: false),
                  borderData: FlBorderData(show: true),
                  gridData: FlGridData(show: true),
                ),
              ),
            ),
    );
  }
}