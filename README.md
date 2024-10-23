# PixKat - Máquina automática de Vendas
Projeto de Vending Machine realizado por Isaac Fiuza Vieira, João Kaszuba, Thassiana Camilia Amorim Muller e Vinicius Henrique Kieling.  
A documentação completa está no PDF "PixKat Documentação"

#### A arquiteta do projeto baseia-se em um modelo usuário, cliente (microcontrolador) e servidor (AWS). O fluxo de funcionamento  entre o usuário, cliente e servidor segue a seguinte sequência:

1 - O usuário se aproxima da vending machine;  
2 - O usuário seleciona a quantidade de produtos desejados ao clicar nos botões “-” e “+” do display;  
3 - O usuário clica no botão “Confirmar” do display da vending machine;  
4 - O microcontrolador envia uma requisição POST com um JSON contendo a quantidade de chocolates selecionada pelo usuário para o serviço de API Gateway da AWS;  
5 - A API invoca uma Lambda que, através de um SDK do Mercado Pago, gera um QR code com o valor a ser pago e um id para acompanhar o status de pagamento;  
6 - O microcontrolador recebe o id da transação e o QR code do pagamento a ser realizado;  
7 - O microcontrolador exibe o QR code do valor a ser pago no display;  
8 - O usuário escaneia o QR code com o aplicativo do Prestador de Serviços Bancários (PSP) de sua preferência e realiza o pagamento;  
9 - O microcontrolador envia uma requisição POST com um JSON contendo o id de pagamento para o serviço de API Gateway da AWS;  
10 - A API invoca uma Lambda que, através de um SDK do Mercado Pago, confirma o pagamento realizado;  
11 - O microcontrolador recebe a confirmação do pagamento e aciona o servo motor que dispensa a quantidade de chocolates pagos.  

#### A figura apresenta um modelo esquemático do fluxo de funcionamento de interação entre as partes do projeto: usuário, cliente e servidor, que foi explicado anteriormente.


![image](https://github.com/user-attachments/assets/5eeb414c-5bba-4f0c-b267-33001c40e6b4)


#### A Modelagem 3D para a impressão do sistema:


![image](https://github.com/user-attachments/assets/3e88ce11-cfb9-4eec-9631-7ac6ddaf6f5a)


#### Imagem do protótipo


![image](https://github.com/user-attachments/assets/b451da7a-0a7a-4867-994a-69b8e7312315)



![image](https://github.com/user-attachments/assets/65bcdd15-eb5c-4107-9076-8f992bc551cc)


