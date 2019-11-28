# Controle-de-Exaustores pela Rede com sensores de temperatura
Sistema de acionamento de exaustores pela rede com monitoramento de temperatura
Este projeto contempla:
 * 1 Módulo Cliente:  - Conecta com a rede
  - Coleta os dados dos sensores (Interno e Externo) e Humidade
  - Monta um pacote udp com as temperaturas e humidade em um protocolo pré-definido
  - Pisca um led azul indicanto o envio do pacote
 * 1 Módulo Servidor:
  - Recebe as informações das temperaturas
  - Roda o Web Service de controle
  - Executa as rotinas escolhidas pelo usuário
