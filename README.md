# bengala-inteligente-iot
Código-Fonte e documentação para a Bengala Inteligente com ESP32, MPU6050 e Sensor Ultrassônico. 

# Projeto da Bengala Inteligente - Práticas IoT

Este repositório contém o código-fonte e a documentação técnica para o desenvolvimento da **Bengala Inteligente**, um dispositivo assistivo de IoT projetado para auxiliar na mobilidade e segurança de pessoas com deficiência visual, além de monitorar e alertar sobre possíveis quedas.

---
## Arquitetura do Sistema e Camadas

O projeto foi estruturado seguindo as principais camadas de uma solução de Internet das Coisas (IoT):

### 1. Camada de Percepção (Hardware)
Esta camada é responsável por coletar os dados do ambiente e do usuário por meio de sensores e fornecer feedbacks físicos através de atuadores:
* **Microcontrolador:** ESP32 WROVER, responsável pelo processamento local e conectividade.
* **Sensor Ultrassônico (HC-SR04):** Utilizado para detectar obstáculos à frente do usuário a uma distância segura.
* **Sensor MPU6050 (Acelerômetro e Giroscópio):** Responsável por monitorar a inclinação e movimentos bruscos para identificar quedas do usuário.
* **Atuadores de Feedback:**
  * **Buzzer:** Alertas sonoros diferenciados para detecção de obstáculos e situações de emergência.

### 2. Camada de Rede e Comunicação
Responsável pelo transporte dos dados coletados para os serviços em nuvem:
* **Conectividade:** Wi-Fi integrado do ESP32 para comunicação com a rede local.
* **Protocolo de Mensageria:** Envio direto de requisições HTTPS para APIs externas.
* **Notificação de Emergência:** Integração com a API do **Telegram** para disparar alertas automáticos com mensagens de socorro para contatos cadastrados assim que uma queda é detectada pelo MPU6050.

### 3. Camada de Serviço e Aplicação (Nuvem e Dashboard)
Engloba o armazenamento de dados e a interface visual para acompanhamento:
* **Banco de Dados:** Armazenamento em nuvem para registrar o histórico de acionamentos e logs de eventos da bengala.
* **Dashboard (Interface Gráfica):** Desenvolvido em **Python** (utilizando bibliotecas como Dash ou Streamlit) para monitoramento em tempo real dos status dos sensores e gráficos de proximidade.

---

## Como Executar o Projeto

### Pré-requisitos
* Arduino IDE instalada e configurada para a placa ESP32.
* Python 3.x instalado na máquina.
* Bibliotecas necessárias para o ESP32 (`Adafruit MPU6050`, `UniversalTelegramBot`, etc.).

### Passos
1. Clone este repositório.
2. Abra o código do ESP32 na pasta correspondente, configure as credenciais do seu Wi-Fi e o Token do seu Bot do Telegram.
3. Faça o upload do código para a placa.
4. Execute o script do Dashboard em Python para visualizar os dados.
