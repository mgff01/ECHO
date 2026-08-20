// --- PINOS DOS SENSORES ULTRASSÔNICOS ---
const int trigPinEsq = 6;
const int echoPinEsq = 7;

const int trigPinFrente = 4;
const int echoPinFrente = 5;

const int trigPinDir = 12;
const int echoPinDir = 11;

// --- PINOS DOS MOTORES E BUZZER ---
const int motorEsqPin = 8;  // <-- MUDOU DO 9 PARA O 8 (Para não dar conflito com o Buzzer!)
const int motorDirPin = 3; 
const int buzzerPin = 22; 

// --- PINOS DOS LEDs (Somente o Vermelho agora) ---
const int ledEsqPin = 44; // Vermelho Esquerda
const int ledDirPin = 2;  // Vermelho Direita

// --- CONFIGURAÇÕES GERAIS ---
const int distanciaMaxima = 30; // Distância onde começa a reagir
const int distanciaMinima = 5;  // Distância de reação máxima

unsigned long ultimoTempoBuzzer = 0;
bool somContinuo = false;

void setup() {
  Serial.begin(9600);
  
  pinMode(trigPinEsq, OUTPUT);
  pinMode(echoPinEsq, INPUT);
  pinMode(trigPinFrente, OUTPUT);
  pinMode(echoPinFrente, INPUT);
  pinMode(trigPinDir, OUTPUT);
  pinMode(echoPinDir, INPUT);
  
  pinMode(motorEsqPin, OUTPUT);
  pinMode(motorDirPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  
  pinMode(ledEsqPin, OUTPUT); 
  pinMode(ledDirPin, OUTPUT); 
  
  analogWrite(motorEsqPin, 255); 
  analogWrite(motorDirPin, 255); 
  
  analogWrite(ledEsqPin, 0);
  analogWrite(ledDirPin, 0);
}

void loop() {
  // 1. Lê a distância dos três sensores
  int distEsq = medirDistancia(trigPinEsq, echoPinEsq);
  int distFrente = medirDistancia(trigPinFrente, echoPinFrente);
  int distDir = medirDistancia(trigPinDir, echoPinDir);
  
  if (distEsq == 0) distEsq = 999;
  if (distFrente == 0) distFrente = 999;
  if (distDir == 0) distDir = 999;

  // --- IMPRIMINDO NO MONITOR SERIAL ---
// --- IMPRIMINDO DADOS NO FORMATO JSON PARA O NEXT.JS ---
  // Vai sair assim: {"E": 15, "F": 30, "D": 8}
  Serial.print("{\"E\":");
  Serial.print(distEsq == 999 ? 30 : distEsq); // Se der erro, joga para 30cm (longe)
  Serial.print(",\"F\":");
  Serial.print(distFrente == 999 ? 30 : distFrente);
  Serial.print(",\"D\":");
  Serial.print(distDir == 999 ? 30 : distDir);
  Serial.println("}");
  // ----------------------------------------------

  // 2. Calcula distância efetiva (Lateral + Frente)
  int distEfetivaEsq = min(distEsq, distFrente);
  int distEfetivaDir = min(distDir, distFrente);

  // 3. LÓGICA DO LADO ESQUERDO
  if (distEfetivaEsq <= distanciaMaxima) {
    int pwmMotorEsq = map(distEfetivaEsq, distanciaMinima, distanciaMaxima, 0, 255);
    analogWrite(motorEsqPin, constrain(pwmMotorEsq, 0, 255));
    
    int brilhoLedEsq = map(distEfetivaEsq, distanciaMinima, distanciaMaxima, 255, 0);
    analogWrite(ledEsqPin, constrain(brilhoLedEsq, 0, 255));
  } else {
    analogWrite(motorEsqPin, 255); 
    analogWrite(ledEsqPin, 0);     
  }

  // 4. LÓGICA DO LADO DIREITO
  if (distEfetivaDir <= distanciaMaxima) {
    int pwmMotorDir = map(distEfetivaDir, distanciaMinima, distanciaMaxima, 0, 255);
    analogWrite(motorDirPin, constrain(pwmMotorDir, 0, 255));
    
    int brilhoLedDir = map(distEfetivaDir, distanciaMinima, distanciaMaxima, 255, 0);
    analogWrite(ledDirPin, constrain(brilhoLedDir, 0, 255));
  } else {
    analogWrite(motorDirPin, 255); 
    analogWrite(ledDirPin, 0);     
  }

  // 5. LÓGICA DO BUZZER
  int menorDistanciaGeral = min(distEsq, min(distFrente, distDir));
  unsigned long tempoAtual = millis(); 

  if (menorDistanciaGeral <= distanciaMinima) {
    if (!somContinuo) {
      tone(buzzerPin, 500); 
      somContinuo = true;
    }
  } 
  else if (menorDistanciaGeral <= distanciaMaxima) {
    if (somContinuo) {
      noTone(buzzerPin); 
      somContinuo = false;
    }
    int intervaloBipe = map(menorDistanciaGeral, distanciaMinima, distanciaMaxima, 100, 800);
    if (tempoAtual - ultimoTempoBuzzer >= intervaloBipe) {
      tone(buzzerPin, 500, 80); 
      ultimoTempoBuzzer = tempoAtual; 
    }
  } 
  else {
    if (somContinuo) {
      noTone(buzzerPin);
      somContinuo = false;
    }
  }

  delay(30); 
}

// --- FUNÇÃO PARA MEDIR A DISTÂNCIA ---
int medirDistancia(int pinoTrig, int pinoEcho) {
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  
  long duracao = pulseIn(pinoEcho, HIGH, 20000); 
  
  if (duracao == 0) return 0; 
  
  int distanciaCalculada = duracao * 0.034 / 2;
  return distanciaCalculada;
}