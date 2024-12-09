// ignore_for_file: avoid_print

import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('PetFeeder')),
        body: MaterialApp(home: MealTimePicker()),
      ),
    );
  }
}

class MealTimePicker extends StatefulWidget {
  @override
  _MealTimePickerState createState() => _MealTimePickerState();
}

class _MealTimePickerState extends State<MealTimePicker> {
  TimeOfDay? _breakfastTime;
  TimeOfDay? _lunchTime;
  TimeOfDay? _dinnerTime;

  final TextEditingController _breakfastController = TextEditingController();
  final TextEditingController _lunchController = TextEditingController();
  final TextEditingController _dinnerController = TextEditingController();

  Future<void> _selectTime(BuildContext context, String meal) async {
    final TimeOfDay? picked = await showTimePicker(
      context: context,
      initialTime: setTime(meal),
      initialEntryMode:
          TimePickerEntryMode.inputOnly, // Use text input mode by default
    );

    if (picked != null) {
      setState(() {
        switch (meal) {
          case 'breakfast':
            _breakfastTime = picked;
            break;
          case 'lunch':
            _lunchTime = picked;
            break;
          case 'dinner':
            _dinnerTime = picked;
            break;
        }
      });
    }
    print(_breakfastTime);
  }

  Widget _buildTimePicker(String label, TimeOfDay? time, String meal) {
    return GestureDetector(
        onTap: () => _selectTime(context, meal),
        child: Container(
          margin: const EdgeInsets.symmetric(vertical: 10.0),
          padding: const EdgeInsets.symmetric(vertical: 5.0, horizontal: 5.0),
          decoration: BoxDecoration(
            border: Border.all(color: Colors.grey),
            borderRadius: BorderRadius.circular(5.0),
          ),
          child: Row(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text(
                '$label: ${time != null ? time.format(context) : "Select Time"}',
                style: const TextStyle(fontSize: 16),
              ),
            ],
          ),
        ));
  }

  @override
  Widget build(BuildContext context) {
    final formKey = GlobalKey<FormState>();
    return Scaffold(
      key: formKey,
      appBar: AppBar(title: const Text('Meal Time Picker'), centerTitle: true),
      body: Padding(
        padding: const EdgeInsets.all(20.0),
        child: Column(
          children: [
            _buildTimePicker('Breakfast', _breakfastTime, 'breakfast'),
            _buildTimePicker('Lunch', _lunchTime, 'lunch'),
            _buildTimePicker('Dinner', _dinnerTime, 'dinner'),
            ElevatedButton(
                onPressed: () {
                  if (_breakfastTime == null ||
                      _lunchTime == null ||
                      _dinnerTime == null) {
                    ScaffoldMessenger.of(context).showSnackBar(
                        const SnackBar(content: Text('Select meal times')));
                    return;
                  }
                  _breakfastController.text =
                      '${_breakfastTime!.hour.toString().padLeft(2, '0')}:${_breakfastTime!.minute.toString().padLeft(2, '0')}';
                  _lunchController.text =
                      '${_lunchTime!.hour.toString().padLeft(2, '0')}:${_lunchTime!.minute.toString().padLeft(2, '0')}';
                  _dinnerController.text =
                      '${_dinnerTime!.hour.toString().padLeft(2, '0')}:${_dinnerTime!.minute.toString().padLeft(2, '0')}';

                  print(_breakfastController.text);

                  var list = [
                    _breakfastController.text,
                    _lunchController.text,
                    _dinnerController.text
                  ];

                  sendData(list);
                },
                child: const Text('Submit'))
          ],
        ),
      ),
    );
  }

  Future<void> sendData(list) async {
    const String esp32Url = 'http://10.0.0.198';

    final Map<String, String> params = {
      'breakfast': list[0],
      'lunch': list[1],
      'dinner': list[2]
    };

    final Uri uri = Uri.parse(esp32Url).replace(queryParameters: params);
    try {
      final response = await http.post(uri);

      if (response.statusCode == 200) {
        print('ESP32 Response: ${response.body}');
      } else {
        print('Failed to send data');
      }
    } catch (e) {
      print('Error: $e');
    }
  }

  TimeOfDay setTime(String meal){
    switch(meal) {
      case 'breakfast':
        return const TimeOfDay(hour: 6, minute: 0);
      case 'lunch':
        return const TimeOfDay(hour: 12, minute: 0);
      case 'dinner':
        return const TimeOfDay(hour: 18, minute: 0);
      default:
        return TimeOfDay.now();
    }
  }
}
