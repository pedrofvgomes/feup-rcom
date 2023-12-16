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

### Experiência 1 - Configure an IP Network
O objetivo desta experiência consiste na configuração dos endereços IP dos computadores Tux23 e Tux24, ligados a um switch.
Após a configuração, pretende-se averiguar o funcionamento do protocolo ARP, nomeadamente quando se eliminam os endereços configurados nas tabelas ARP.

![Imagem 1](img/image1.png){style="display: block; margin: 0 auto; border: solid 1px black"}
<p align='center'>Imagem 1</p>

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

### Experiência 3 - Configure a Router in Linux

### Experiência 4 - Configure a Commercial Router and Implement NAT

### Experiência 5 - DNS

### Experiência 6 - TCP connections

