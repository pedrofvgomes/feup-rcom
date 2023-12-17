# Lab 2 - Redes de Computadores

## Índice
- [Parte 1 - FTP Downloader](#parte-1---ftp-downloader)
- [Parte 2 - Configuration and Study of a Network](#parte-2---configuration-and-study-of-a-network)
    - [Limpar configurações](#limpar-configurações)
    - [Experiência 1 - Configure an IP Network](#experiência-1---configure-an-ip-network)
    - [Experiência 2 - Implement two bridges in a switch](#experiência-2---implement-two-bridges-in-a-switch)
    - [Experiência 3 - Configure a Router in Linux](#experiência-3---configure-a-router-in-linux)
    - [Experiência 4 - Configure a Commercial Router and Implement NAT](#experiência-4---configure-a-commercial-router-and-implement-nat)
    - [Experiência 5 - DNS](#experiência-5---dns)
    - [Experiência 6 - TCP connections](#experiência-6---tcp-connections)


## Parte 1 - FTP Downloader

## Parte 2 - Configuration and Study of a Network

***Nota**: Estas experiências foram executadas na bancada 2, pelo que os nomes dos computadores (Tux21, Tux22, ...) serão relativos à bancada de cada um (bancada Y - TuxY1, TuxY2, ...).*

### Limpar configurações
Para iniciar as experiências, deveremos primeiro limpar as configurações do switch.
Para isto, teremos de ligar a porta S0 do Tux23, por exemplo, à porta T3, e a porta *Switch Console* à porta T4. Desta forma, esse computador terá acesso ao terminal do Switch.
Depois, abrimos o GKTerm, na porta /dev/ttyS0 e alteramos o *baudrate* para 115200, e pressionamos Enter, para termos acesso ao terminal.
Por fim, a seguinte sequência de comandos reiniciará a configuração do switch, para darmos início às experiências.
```bash
admin
/system reset-configuration
y
```

### Experiência 1 - Configure an IP Network
O objetivo desta experiência consiste na configuração dos endereços IP dos computadores Tux23 e Tux24, ligados a um switch.
Após a configuração, pretende-se averiguar o funcionamento do protocolo ARP, nomeadamente quando se eliminam os endereços configurados nas tabelas ARP.

<p align='center'>
    <img src="img/image1.png"/><br>
    Imagem 1
</p>

1. #### Disconnect the switch from netlab (PY.1). Connect tuxY3 and tuxY4 to the switch
Este passo inicial consiste em desconectar o switch da entrada P2.1, e posteriormente conectar as portas E0 do Tux23 e Tux24 nas portas 1 e 2 do switch. 
Devemos verificar sempre que a luz das portas acendeu, mostrando que o cabo está corretamente ligado.

2. #### Configure tuxY3 and tuxY4 using ifconfig and route commands
Seguindo a imagem fornecida pelo guião (Imagem 1), vemos que teremos de associar os endereços `172.16.20.1/24` ao Tux23 e `172.16.20.0.254/24` ao Tux24.
Para isto, iremos ao terminal e executaremos estes comandos:
- No Tux23:
```bash
ifconfig eth0 up
ifconfig eth0 172.16.20.1/24
``` 
- No Tux24:
```bash
ifconfig eth0 up
ifconfig eth0 172.16.20.254/24
``` 

3. #### Register the IP and MAC addresses of network interfaces
Para registar os endereços MAC, executaremos ```ifconfig``` no terminal, em cada um dos computadores que acabámos de usar, e os endereços MAC estarão na entrada ```ether``` do output.

4. #### Use ping command to verify connectivity between these computers
Para verificar que a ligação foi corretamente estabelecida, deveremos executar ```ping 172.16.20.1``` (Tux24) e ```ping 172.16.20.254``` (Tux24). 
Estes comandos irão, inicialmente, gerar pacotes ARP. Quando encontrarem o endereço MAC da máquina de destino, enviarão pacotes do tipo ICMP (Internet Control Message Protocol) para transferência de informação.

5. #### Inspect forwarding (route -n) and ARP (arp -a) tables
Usando o comando ```route -n```, no Tux23, receberemos um output que associa o endereço IP à porta correspondente.
Posteriormente, o comando ```arp -a``` mostrar-nos-á o endereço MAC correspondente a esse mesmo endereço IP.
Output:
```bash
?(172.16.50.254) at 00:21:5a:c3:78:70 [ether] on eth0
```

6. #### Delete ARP table entries in tuxY3 (arp -d ipaddress)
Neste passo, executaremos o comando ```arp -d 172.16.20.254``` para apagarmos da tabela ARP a entrada correspondente a este endereço IP.
Para verificação, se executarmos ```arp -a``` novamente, o output será vazio.

7. #### Start Wireshark in tuxY3.eth0 and start capturing packets
Teremos de abrir o Wireshark - será aqui que veremos os pacotes que são enviados e recebidos.

8. #### In tuxY3, ping tuxY4 for a few seconds
Executando ```ping 172.16.20.254/24``` no Tux23, os pacotes que são enviados estarão representados nos logs do Wireshark.

9. #### Stop capturing packets
```ctrl + C``` no terminal.

10. #### Save the log and study it at home
(Logs do Wireshark).

### Experiência 2 - Implement two bridges in a switch
O objetivo desta experiência consiste na implementação de duas *bridges* no switch, criando assim duas LANs (Local Area Network).

<p align='center'>
    <img src="img/image2.png"/><br>
    Imagem 2
</p>

1. #### Connect and configure tuxY2 and register its IP and MAC addresses
Conforme a imagem indica, teremos de configurar o Tux22 no endereço IP `172.16.21.1`.
Para isso, teremos de ligar a porta E0 do Tux22 ao switch, e nesse computador executaremos os seguintes comandos:
```bash
ifconfig eth0 up
ifconfig eth0 172.16.21.1/24
ifconfig eth0
```
Este último comando servirá para verificar se o endereço ficou corretamente configurado - o output deverá conter o endereço correto, e teremos de registar o endereço MAC que estará na entrada `ether` do output.

2. #### Create two bridges in the switch: bridgeY0 and bridgeY1
Observando a imagem, vemos que a bridge20 conterá o Tux23 e Tux24, e a bridge21 conterá apenas o Tux22.
Assim, criaremos as duas bridges com os seguintes comandos:
```bash
/interface bridge add name=bridge20
/interface bridge add name=bridge21
```

3. #### Remove the ports where tuxY3, tuxY4 and tuxY2 are connected from the default bridge (bridge) and add them the corresponding ports to bridgeY0 and bridgeY1
Para eliminar as portas que estão, por defeito, ligadas nos computadores, executaremos os seguintes comandos:
```bash
/interface bridge port remove [find interface=ether1]
/interface bridge port remove [find interface=ether2]
/interface bridge port remove [find interface=ether3]
```
E para terminar a configuração das bridges, usaremos os seguintes comandos.
```bash
/interface bridge port add bridge=bridge20 interface=ether1
/interface bridge port add bridge=bridge20 interface=ether2
/interface bridge port add bridge=bridge20 interface=ether3
```
Para verificar se as bridges foram corretamente implementadas, usaremos o seguinte comando:
```bash
/interface bridge port print
```

4. #### Start the capture at tuxY3.eth0
No Tux23, abriremos o Wireshark para fazer a captura dos pacotes para os seguintes passos.

5. #### In tuxY3, ping tuxY4 and then ping tuxY2
Ainda no Tux23, usaremos os seguintes comandos para fazer ping do Tux24 e Tux22, respetivamente. 
***Nota**: Deveremos observar os logs do Wireshark após cada ping, para verificar se os pacotes estão a ser corretamente enviados.*
```bash
ping 172.16.20.254
ping 172.16.21.1
```

Conclui-se que o ping em Tux24 foi corretamente efetuado, mas não no Tux22. Isto deve-se ao facto de o primeiro estar ligado ao Tux23 pela bridge20, mas o segundo encontra-se numa bridge diferente, logo não há forma de fazer a ligação.

6. #### Stop the capture and save the log
```ctrl + C```
(Logs do Wireshark).

7. #### Start new captures in tuxY2.eth0, tuxY3.eth0, tuxY4.eth0
Executar os seguintes comandos em cada um dos computadores:
```bash
ping 172.16.20.255
ping 172.16.21.255
```


8. #### In tuxY3, do ping broadcast (ping -b 172.16.Y0.255) for a few seconds
Autoexplicativo

9. #### Observe the results, stop the captures and save the logs
(Logs do Wireshark).

10. #### Repeat steps 7, 8 and 9, but now do ping broadcast in tuxY2 (```ping -b 172.16.Y1.255```)
Autoexplicativo.


### Experiência 3 - Configure a Router in Linux
O objetivo desta experiência será usar o Tux24 como router, usando as bridges criadas anteriormente, para que os computadores Tux22 e Tux23 possam comunicar entre si.
O Tux24 servirá como *gateway*.

<p align='center'>
    <img src="img/image3.png"/><br>
    Imagem 3
</p>


1. #### Transform tuxY4 (Linux) into a router
Como a porta E0 do Tux24 já foi configurada, teremos de usar a E1.
Para isso, ligaremos essa porta a uma nova entrada do switch, e teremos de configurar o endereço IP usando os seguintes comandos:
```bash
ifconfig eth1 up
ifconfig eth1 172.16.21.253/24
```
Agora, teremos de ligar esta porta à bridge21. Desta forma, o Tux23 está ligado pela bridge20 ao Tux24, que por sua vez estará ligado pela bridge21 ao Tux22, desta forma assegurando a comunicação entre os três computadores.
Primeiro, teremos de eliminar as portas às quais o Tux24 está ligado por defeito, e adicionar a nova porta, usando os seguintes comandos:

```bash
/interface bridge port remove [find interface=ether4]
/interface bridge port add bridge=bridge21 interface=ether4
```

Por fim, ainda no Tux24, teremos de ativar IP forwarding e ativar ICMP, usando os seguintes comandos:

```bash
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts
```

2. #### Observe MAC addresses and IP addresses in tuxY4.eth0 and tuxY4.eth1
Para este passo, usaremos o comando ```ifconfig``` no Tux24 para registar os endereços IP e MAC de ```eth0``` e ```eth1```.

3. #### Reconfigure tuxY3 and tuxY2 so that each of them can reach the other
Neste passo, teremos de usar comandos ```route``` para que o Tux22 e Tux23 tenham accesso ao router. 
Como podemos ver pela imagem, o Tux22 está ligado ao Tux24 por ```172.16.21.0/24``` e o Tux23 está ligado ao Tux24 por ```172.16.20.0/24```, e o Tux24 está nas portas 253 e 254 destes endereços, respetivamente.
A configuração que faremos, para cada um dos computadores, terá em consideração o endereço que pretendemos aceder, e a porta onde o Tux24 se encontra no endereço atual.
Logo, os comandos a utilizar serão:
- Tux22:
```bash
route add -net  172.16.50.0/24 gw 172.16.51.253
```
- Tux23:
```bash
route add -net  172.16.51.0/24 gw 172.16.51.254
```

4. #### Observe the routes available at the 3 tuxes (route -n)
Para verificar se as rotas foram bem configuradas, usaremos o comando ```route -n```.

5. #### Start capture at tuxY3
Abrir o Wireshark no Tux23 para os passos seguintes.

6. #### From tuxY3, ping the other network interfaces (172.16.Y0.254, 172.16.Y1.253, 172.16.Y1.1) and verify if there is connectivity
Executar os seguintes comandos:
```bash
ping 172.16.20.254
ping 172.16.20.253
ping 172.16.20.1
```
Se estiver tudo feito corretamente até agora, os pings devem estar representados no Wireshark.

7. #### Stop the capture and save the log
(Logs do Wireshark).

8. #### Start capture in tuxY4; use 2 instances of Wireshark, one per network interface
Abrir duas janelas do Wireshark, uma para o `eth0` e uma para o `eth1`.

9. #### Clean the ARP tables in the 3 tuxes
- Tux22:
```bash
arp -d 172.16.21.253
```
- Tux23:
```bash
arp -d 172.16.20.254
```
- Tux24:
```bash
arp -d 172.16.20.1
arp -d 172.16.21.1
```
Isto eliminará as tabelas ARP do Tux24, no Tux22 e no Tux23, e dos Tux22 e Tux23, no Tux24.

10. #### In tuxY3, ping tuxY2 for a few seconds.
No Tux23, executar o seguinte comando:
```bash
ping 172.16.21.1
```
Isto acederá ao Tux22 através do router Tux24.

11. #### Stop captures in tuxY4 and save logs
```ctrl + C```

(Logs do Wireshark).


### Experiência 4 - Configure a Commercial Router and Implement NAT

### Experiência 5 - DNS

### Experiência 6 - TCP connections

