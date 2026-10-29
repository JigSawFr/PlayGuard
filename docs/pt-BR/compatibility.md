<!-- translated from docs/compatibility.md, sha256 74e7e81f48cb -->
# Compatibilidade

Firmwares, Atmosphère e launchers compatíveis, emuMMC e sysMMC, e como o PlayGuard se compara a um sysmodule substituto. Volte ao [README](../../README.md).

## Versões compatíveis

**Testado em um console** — o PlayGuard já rodou nestes:

- **Firmware 23.0.1** — Atmosphère 1.12.1
- **Firmware 22.0.0, 22.1.0, 22.5.0** — Atmosphère 1.11.x
- hbmenu, sphaira e a Homebrew App Store

**Deve funcionar, não testado** — nada no PlayGuard impede, mas ninguém relatou ainda ([relatos são bem-vindos](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)). Cada firmware precisa de um Atmosphère que o inicie: o indicado ou qualquer um mais recente.

- **Firmware 23.0.0** — Atmosphère 1.12.0. Os mesmos comandos do 23.0.1 na tabela do [switchbrew](https://switchbrew.org/wiki/Parental_Control_services).
- **Firmware 21.0.0 → 21.2.0** — Atmosphère 1.10.x. Todos os recursos.

**Antes do 21.0.0, tudo menos o temporizador** — restrições, PIN, desbloqueio temporário, atividade, relógio de rede, app do celular e histórico funcionam. O bloco de configurações do temporizador (0x44 bytes) só tem esta estrutura desde o 21.0.0, então o PlayGuard desativa o que depende dele: os limites diários, o alarme de hora de dormir, o tempo extra e *Bloquear todos os jogos*.

- **Firmware 20.0.0 → 20.5.0** — Atmosphère 1.9.x
- **Firmware 19.0.x** — Atmosphère 1.8.0
- **Firmware 18.0.0 → 18.1.0** — Atmosphère 1.7.x
- **Firmware 17.0.x** — Atmosphère 1.6.x
- **Firmware 16.0.0 → 16.1.0** — Atmosphère 1.5.x
- **Firmware 15.0.x** — Atmosphère 1.4.x
- **Firmware 14.x** — Atmosphère 1.3.x
- **Firmware 13.x** — Atmosphère 1.1.0 → 1.2.6
- **Firmware 12.x e anteriores** — Atmosphère 1.0.0 e anteriores

O PlayGuard verifica se o Atmosphère está rodando, não qual versão: o que conta é o firmware que ele inicia. *Console* mostra os dois.

## Bom saber

### Launchers

É recomendado abrir por cima de um jogo (title override). O app indica se roda como aplicativo (por cima de um jogo, ou pelo seu próprio forwarder no menu HOME) ou como applet (álbum). Por cima de um jogo, o console conta o tempo no PlayGuard como tempo desse jogo. Na atividade, ele vai para o usuário escolhido ao abrir: abra com o usuário de um responsável, não com o de uma criança. O temporizador é do console, o mesmo para todos os usuários: ele conta esse tempo seja quem for que abriu o PlayGuard, a menos que o temporizador esteja desativado ou que o PIN tenha pausado o controle parental ([detalhes](features.md#o-pin-pausa-o-controle-parental-enquanto-você-está-no-playguard)).

### Nintendo Switch 2

Não é compatível: não existe Atmosphère para ele.

### Firmware mais recente

Acima do firmware mais recente testado em um console, o PlayGuard abre **somente leitura** e verifica se uma versão mais nova oferece suporte a ele. Se houver uma, ele oferece a atualização (no próprio PlayGuard, pelo sphaira ou pela Homebrew App Store). Se não, você escolhe: somente leitura, somente leitura com as ferramentas de desenvolvedor (para investigar o firmware) ou todos os recursos por sua conta e risco. A escolha pode ser lembrada para esse firmware e essa versão do app; *Console › Compatibilidade* mostra a tela de novo.

### emuMMC e sysMMC

Cada um tem seu próprio controle parental, e o PlayGuard só vê o do sistema em que roda. No emuMMC, *Console* e *Primeiros passos* avisam isso; no sysMMC também, quando há um emuMMC configurado no cartão SD. Configure o controle parental nos dois, ou esconda o menu de inicialização do hekate para que uma criança não possa escolher o outro sistema. O PlayGuard guarda separados o histórico, os backups e o estado de cada sistema (o sysMMC e o emuMMC, em `nand/`); as preferências e os perfis são compartilhados.

### O console pode ser banido?

O PlayGuard nunca contata a Nintendo: online, ele só acessa servidores de horário públicos, o GitHub (atualizações e os relatórios que você envia) e o switchbrew (o firmware mais recente e suas notas de versão). O que faz um console ser banido é um firmware personalizado ou um emuMMC se conectando à Nintendo ([FAQ do switchbrew](https://switchbrewdocs.readthedocs.io/en/latest/faq.html), em inglês). Nenhum banimento ligado a mudanças no controle parental ou no relógio está documentado; isso não é uma garantia.

### Um console sem internet

É para isso que o PlayGuard existe, e o temporizador ainda precisa de um relógio de rede ajustado. Sem servidor de horário para consultar, *Console › Relógio de rede › Medir a diferença de horário* oferece ajustá-lo pelo relógio do console: confira primeiro o relógio do console, com *Sincronizar relógio pela Internet* desativado, e só ative essa configuração depois (veja [relógio de rede](features.md#nenhum-servidor-responde)).

### Notificações no celular

Desde o 22.0.0, um console ainda vinculado ao app do celular pode avisar o responsável quando o PIN é digitado no console ([switchbrew](https://switchbrew.org/wiki/22.0.0), em inglês). Não se sabe se os desbloqueios do próprio PlayGuard disparam esse aviso.

## Por que o PlayGuard

As crianças adoram videogame, e videogame significa tela: um limite faz parte de cuidar delas. Em um Switch sem modificação, o app de celular da Nintendo cuidava disso. Quando o console passa a rodar um firmware personalizado, um mundo novo se abre — e o controle parental vai embora, porque o app do celular não alcança mais o console. Saber quanto tempo elas jogaram, definir um limite, fazer o jogo parar sem briga nem mais um "só mais cinco minutos" ficou difícil.

As poucas ferramentas que existiam não davam conta: relatórios de atividade pouco mantidos e pouco detalhados, controles parentais rudimentares e inacabados. E eu também não queria um substituto caseiro, nem um sysmodule sempre rodando em segundo plano. A ideia era **reaproveitar ao máximo o controle parental do próprio console**, com tudo o que ele já faz — o limite, o PIN, os avisos, a suspensão — e trazer as configurações dele de volta para o console. Isso é o PlayGuard.

Comandar o controle parental do próprio console também abre a porta para muito mais: enviar o tempo de jogo para um servidor em casa, receber ordens dele, Home Assistant, automações. O primeiro passo já está aqui, uma conexão opcional com o Home Assistant por MQTT ([guia](../home-assistant.md), em inglês); para onde isso pode ir depois está no [horizonte do roadmap](../../ROADMAP.md#horizon) (em inglês).

## PlayGuard ou um sysmodule substituto?

O PlayGuard comanda o controle parental embutido no console (o serviço `pctl`), offline. A outra abordagem o substitui por um sysmodule, por exemplo o [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (um sysmodule e um overlay Tesla / Ultrahand).

| | PlayGuard | Um sysmodule substituto |
|---|---|---|
| **Limite de tempo de jogo** | Um para o console (o temporizador do próprio console) | Um por conta de usuário |
| **PIN, avisos, suspensão** | Os do console | Os dele |
| **Em segundo plano** | Nada (o sysmodule de recuperação opcional só age na inicialização, quando um arquivo `RESCUE` pede; o agente da conexão com o Home Assistant, também opcional, só roda depois de instalado) | Um sysmodule, desde a inicialização |
| **Depois de uma atualização do firmware** | O controle do console continua funcionando; o PlayGuard abre somente leitura até uma versão oferecer suporte ao firmware | O controle depende de o sysmodule continuar funcionando |
| **Apagado do cartão SD** | Os limites continuam no console | O controle vai embora com ele |

**Precisa de um limite diferente para cada criança?** Escolha um sysmodule substituto — o temporizador do console tem um único limite para o console inteiro. Caso contrário, o PlayGuard mantém o controle parental da Nintendo e só traz as configurações dele para o console.
