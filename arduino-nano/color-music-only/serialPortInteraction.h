void sendDataType(String &dataType) {
  Serial.print(F("{\"dataType\":\""));
  Serial.print(dataType);
  Serial.print(F("\",\"data\":"));
}

void sendJsonArray(uint8_t *arr, const uint16_t &arrSize, String dataType = "null") {
  sendDataType(dataType);
  Serial.print("[" + String(arr[0]));
  for (uint16_t i = 1; i < arrSize;) Serial.print("," + String(arr[i++]));
  Serial.println("]}");
}

void sendJsonArray(int16_t *arr, const uint16_t &arrSize, String dataType = "null") {
  sendDataType(dataType);
  Serial.print("[" + String(arr[0]));
  for (uint16_t i = 1; i < arrSize;) Serial.print("," + String(arr[i++]));
  Serial.println("]}");
}


void sendSamples(SamplingConfigs &sampling) {
  sendJsonArray(sampling.samples, sampling.samplesLength, "samplesData");
}

void sendAmplitudes(FHTData &fhtData) {
  sendJsonArray(fhtData.amplitudes, fhtData.amplitudesLength, "fhtData");
}

void sendLowHistory(History &history) {
  sendJsonArray(history.data, LENGTH_COLOR_AMPLITUDES, "lowHistory");
}

void sendMiddleHistory(History &history) {
  sendJsonArray(history.data, LENGTH_COLOR_AMPLITUDES, "middleHistory");
}

void sendHighHistory(History &history) {
  sendJsonArray(history.data, LENGTH_COLOR_AMPLITUDES, "highHistory");
}

void sendLeds(Leds &leds) {
  Serial.print(F("{\"dataType\":\"ledData\",\"data\":"));
  Serial.print("[" + String(leds.red));
  Serial.print("," + String(leds.green));
  Serial.print("," + String(leds.blue));
  Serial.println("]}");
}

void sendSamplingConfigs(SamplingConfigs &sampling, String dataType = "configs") {
  sendDataType(dataType);
  Serial.print(F("{\"audioPin\":")); Serial.print(String(sampling.audioPin));
  Serial.print(F(",\"delayReadingSamplesUs\":")); Serial.print(String(sampling.delayReadingSamplesUs));
  Serial.print(F(",\"offsetReadingSamples\":")); Serial.print(String(sampling.offsetReadingSamples));
  Serial.print(F(",\"deltaSignal\":")); Serial.print(String(sampling.samplesAmplitude));
  Serial.print(F(",\"medianOfSamplesAmplitude\":")); Serial.print(String(sampling.medianOfSamplesAmplitude));
  Serial.print(F(",\"samplingFrequrency\":")); Serial.print(String(sampling.samplingFrequrency));
  Serial.println("}.}");
}
