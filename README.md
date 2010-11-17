
 School project: Implementation of Selective Repeat ARQ using UDP in C on Unix
===============================================================================

Course AE4B33OSS at Czech Technical Univerzity, Faculty of Electrical Engineering.


 Assignment (in Czech language)
--------------------------------

> ### 2. Potvrzované spojení
>
> Implementujte potvrzované spojení využívající následující algoritmus:
>
>  a) Go Back N ARQ: Implementujte algoritmus Go back N pro N=8.
>
>  **b) Selective Repeat ARQ: Implementujte algoritmus selective repeat pro N=8.**
>
> Program klient na začátku otevře soketové připojení podle zadaných argumentů (IP a číslo
> portu) a dále vysílá zprávy, které si přečte ze standardního vstupu (zprávy jsou
> odděleny znakem Nová_řádka a jsou ořezány, nebo doplněny na pevnou délku). Při EOF
> ukončí soketové spojení.

> Klient před každou zprávu přidá hlavičku, která bude obsahovat dva bajty sekvenčního
> čísla. Pro posílání zpráv využijte soketu typu Datagram. Všechny posílané zprávy budou
> mít pevnou délku. Program klient podle generátoru náhodných čísel buď zprávu s
> pravděpodobností 0.9 vpořádku odešle, nebo s pravděpodobností 0.1 bude simulovat výpadek
> telegramu (tedy neodešle žádný bajt zprávy).
>
> Program server přijímá telegramy a vysílá potvrzení o jejich přijetí. Server také podle
> generátoru náhodných čísel s pravděpodobností 0.9 potvrzení v pořádku zašle a s
> pravděpodobností 0.1 potvrzení zahodí. Server na standardní výstup zobrazuje přijaté
> zprávy.
>
> Klient se spouští s 2 parametry: IP_serveru, port_serveru.
>
> Server se spouští s 1 parametrem: port_serveru.


 Assignment
------------

> ### 2. Communication with acknowledgment
>
> Create program that implements communication with acknowledgment with one of the
> following algorithms:
>
> a) Go Back N ARQ: Implement algorithm Go back N for N=8.
>
> **b) Selective Repeat ARQ: Implement algorithm selective repeat for N=8.**
>
> The client program opens a socket connection according the program parameters (IP and
> port number) and sends messages from the standard input (messages are separated by new
> line character and are cut or filled by space(s) to fixed length). Character EOF ends
> the socket connection and the program.
>
> Client adds 2 bytes (sequence number) before each message. For sending the message use a
> datagram socket. Every telegram has a fixed length. The client program uses a generator
> of random number to decide whether the telegram will be send or not. Client with the
> probability of 0.9 sends telegram and with the probability of 0.1 will simulate the
> drop-out of the telegram.
>
> The server program receives telegrams from client and sends an acknowledgment. Server
> also uses the generator of random numbers. With the probability of the 0.9 the server
> sends acknowledgment to client and with the probability of 0.1 drops-out the
> acknowledgment. Server shows the received messages on the standard output.
>
> Client is started with 2 parameters: server_IP, server_port.
>
> Server is started with 1 parameter: server_port.

