<!-- translated from docs/compatibility.md, sha256 33251c660faf -->
# Compatibilidade

Firmware, Atmosphère e launchers suportados, emuMMC e sysMMC, e como o PlayGuard se compara com um sysmodule de substituição. Voltar ao [README](../../README.md).

## Versões suportadas

**Testado numa consola** — o PlayGuard foi executado nestes:

- **Firmware 23.0.1** — Atmosphère 1.12.1
- **Firmware 22.0.0, 22.1.0, 22.5.0** — Atmosphère 1.11.x
- hbmenu, sphaira e a Homebrew App Store

**Deve funcionar, não testado** — nada no PlayGuard os exclui, mas ninguém os relatou ainda ([relatos bem-vindos](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)). Cada firmware precisa de um Atmosphère que o arranque: o indicado ou qualquer um mais recente.

- **Firmware 23.0.0** — Atmosphère 1.12.0. Os mesmos comandos que a 23.0.1 na tabela do [switchbrew](https://switchbrew.org/wiki/Parental_Control_services).
- **Firmware 21.0.0 → 21.2.0** — Atmosphère 1.10.x. Todas as funcionalidades.

**Antes da 21.0.0, tudo menos o temporizador de jogo** — restrições, PIN, desbloqueio temporário, atividade, relógio de rede, aplicação móvel e histórico funcionam. O bloco de definições do temporizador de jogo (0x44 bytes) só tem esta estrutura desde a 21.0.0, por isso o PlayGuard desativa o que depende dele: os limites diários, o alarme de hora de dormir, o tempo extra e o bloqueio da consola.

- **Firmware 20.0.0 → 20.5.0** — Atmosphère 1.9.x
- **Firmware 19.0.x** — Atmosphère 1.8.0
- **Firmware 18.0.0 → 18.1.0** — Atmosphère 1.7.x
- **Firmware 17.0.x** — Atmosphère 1.6.x
- **Firmware 16.0.0 → 16.1.0** — Atmosphère 1.5.x
- **Firmware 15.0.x** — Atmosphère 1.4.x
- **Firmware 14.x** — Atmosphère 1.3.x
- **Firmware 13.x** — Atmosphère 1.1.0 → 1.2.6
- **Firmware 12.x e anteriores** — Atmosphère 1.0.0 e anteriores

O PlayGuard verifica que o Atmosphère está a correr, não a sua versão: o que conta é o firmware que ele arranca. *Consola* mostra os dois.

## A saber

### Launchers

Recomenda-se abrir o PlayGuard sobre um jogo (title override). A aplicação diz se está a correr como aplicação ou como applet (álbum). Sobre um jogo, a consola conta o tempo do PlayGuard como tempo desse jogo. Na atividade, vai para o utilizador escolhido ao iniciar: abra-o com o utilizador de um adulto, não de uma criança. O temporizador de jogo é o da consola, o mesmo para todos os utilizadores: conta esse tempo seja quem for que abriu o PlayGuard (a menos que o temporizador esteja desligado).

### Nintendo Switch 2

Não suportada: não existe Atmosphère para ela.

### Firmware mais recente

Acima do firmware mais recente testado numa consola, o PlayGuard abre **só de leitura** e verifica se uma versão mais recente o suporta. Se houver, propõe atualizar (no próprio PlayGuard, através do sphaira ou da Homebrew App Store). Senão, a escolha é sua: só de leitura, só de leitura com as ferramentas de programador (para investigar o firmware), ou todas as funcionalidades por sua conta e risco. A escolha pode ser memorizada para esse firmware e essa versão da aplicação; *Consola › Compatibilidade* volta a mostrar o ecrã.

### emuMMC e sysMMC

Cada um tem o seu próprio controlo parental, e o PlayGuard só vê o do sistema em que corre. No emuMMC, *Consola* e *Primeiros passos* avisam disso; no sysMMC também, quando há um emuMMC configurado no cartão SD. Defina o controlo parental nos dois, ou esconda o menu de arranque do hekate para que uma criança não possa escolher o outro sistema. O PlayGuard mantém separados o histórico, as cópias de segurança e o estado de cada sistema (o sysMMC e o emuMMC, em `nand/`); as preferências e os perfis são partilhados.

### A consola pode ser banida?

O PlayGuard nunca contacta a Nintendo: online, só chega a servidores de hora públicos e ao GitHub (atualizações, e os relatórios que enviar). O que faz banir consolas é um firmware personalizado ou um emuMMC que se liga à Nintendo ([FAQ do switchbrew](https://switchbrewdocs.readthedocs.io/en/latest/faq.html), em inglês). Nenhum ban ligado a alterações do controlo parental ou do relógio está documentado; não é uma garantia.

### Uma consola sem Internet

É para isso que o PlayGuard serve, e o temporizador de jogo continua a precisar de um relógio de rede que tenha sido acertado. Sem servidor de hora a que perguntar, *Consola › Relógio de rede › Medir a diferença horária* propõe acertá-lo pelo relógio da consola: verifique primeiro o relógio da consola, com *Sincronizar relógio através da Internet* desativado, e só depois ative essa definição (veja [relógio de rede](features.md#nenhum-servidor-responde)).

### Notificações no telemóvel

Desde a 22.0.0, uma consola ainda associada à aplicação móvel pode notificar o pai ou a mãe quando o PIN é introduzido na consola ([switchbrew](https://switchbrew.org/wiki/22.0.0), em inglês). Não se sabe se os desbloqueios do próprio PlayGuard o fazem disparar.

## Porquê o PlayGuard

As crianças adoram videojogos, e videojogos são ecrãs: um limite faz parte de cuidar delas. Numa Switch sem modificações, a aplicação móvel da Nintendo tratava disso. Quando a consola passa a correr um firmware personalizado, abre-se um mundo novo — e o controlo parental vai pela janela fora, porque a aplicação móvel deixa de chegar à consola. Saber quanto tempo jogaram, definir um limite, fazer parar o jogo sem uma batalha nem mais um "só mais cinco minutos" tornou-se difícil.

As poucas ferramentas que existiam não resolviam: relatórios de atividade pouco mantidos e pouco detalhados, um controlo parental rudimentar e inacabado. E eu também não queria um substituto feito à medida, nem um sysmodule sempre a correr em segundo plano. A ideia era **reaproveitar ao máximo o controlo parental da própria consola**, com tudo o que ele já faz — o limite, o PIN, os avisos, a suspensão — e trazer as suas definições de volta para a consola. É isso o PlayGuard.

Comandar o controlo parental da própria consola também abre a porta a muito mais: enviar o tempo de jogo para um servidor em casa, receber ordens dele, Home Assistant, automatizações. Até onde isso pode ir está no [horizonte do roteiro](../../ROADMAP.md#horizon) (em inglês).

## PlayGuard ou um sysmodule de substituição?

O PlayGuard comanda o controlo parental integrado na consola (o serviço `pctl`), offline. A outra abordagem substitui-o por um sysmodule, por exemplo o [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl) (um sysmodule e um overlay Tesla / Ultrahand).

| | PlayGuard | Um sysmodule de substituição |
|---|---|---|
| **Limite de tempo de jogo** | Um para a consola (o próprio temporizador da consola) | Um por conta de utilizador |
| **PIN, avisos, suspensão** | Os da consola | Os seus próprios |
| **Em segundo plano** | Nada (o sysmodule de recuperação opcional só atua no arranque, quando um ficheiro `RESCUE` o pede) | Um sysmodule, desde o arranque |
| **Depois de uma atualização do firmware** | O controlo parental da consola continua a funcionar; o PlayGuard abre só de leitura até uma versão suportar o firmware | O controlo depende de o sysmodule continuar a funcionar |
| **Eliminado do cartão SD** | Os limites ficam na consola | O controlo desaparece com ele |

**Precisa de um limite diferente para cada criança?** Escolha um sysmodule de substituição — o temporizador da consola tem um só limite para toda a consola. Caso contrário, o PlayGuard mantém o controlo parental da Nintendo e limita-se a trazer as suas definições para a consola.
