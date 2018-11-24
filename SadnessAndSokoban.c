/*  Jul, 2017
    Dupla: Carlos Morvan e Guilherme Malta
    Informacoes importantes:
        - Optamos por trabalhar com um jogo onde o save funciona através de checkpoints. Ou seja, os dados de um jogador são
            atualizados a partir do momento em que ele completa um nível. Caso ele deseje retornar ao jogo em outro momento,
            o jogador continuará a partir do último nível em que obteve sucesso.
        - Para melhor jogabilidade, fizemos uma pequena modificacao do mapa de nível 1, e implementamos um novo mapa bonus, chamado
            nivel 4. Neste, o jogador deve empurrar as caixas para os lugares corretos (pré-definidos) o mais rapido possível). É
            uma corrida final contra o tempo.
        - há também 3 atributos bônus: um mapa para os lugares corretos, um sistema de "carga" no temporizador do nível, e um sistema de vidas.
            O mapa [d] informa por 2 segundos as coordenadas de onde as caixas devem ser encaixadas. Fica para o jogador descobrir como
            interpretar estas coordenadas. Cabe ao jogador também, entender como funciona o "clock-charge". A cada uso, o jogador ganha
            mais tempo, mas este valor não é fixo. Ele depende do nível em que o jogador se encontra. Com o sistema de vidas, a mesma coisa.
            Isso possibilita que o jogador, com o tempo, entenda melhor o funcionamento destes atributos bônus e desenvolva uma estratégia
            para usá-los nos melhores momentos.
        -[IMPORTANTE] Os .txt com os níveis, devem ser EXATAMENTE os anexados ao .rar. Caso contrário o jogo terá problemas de execução.*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio2.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#define MAXALTURA 10 //para abertura do nivel
#define MAXLARGURA 25 //para abertura do nivel
#define MAXNOME 16 // para abertura do nivel

#define CIMA 72
#define BAIXO 80
#define ESQ 75
#define DIR 77

#define ENTER 13 //ENTER (usado para selecao de menu)
#define TAB 9 //TAB (sera usado para menu)
#define ESC 27 // ESC
#define DICA 100 // d:  Dica de lugar de encaixe(BONUS-FACTOR)
#define CLOCK 115 // s: carga no temporizador (BONUS-FACTOR)

typedef struct highscore_st
{
    /* Estrutura utilzada para salvar a pontuação dos jogadores*/
    char nome_jogador[12];
    float pontuacao;

} Highscore;

typedef struct save_st
{
    /*estrutura utilizada para salvar o estado atual do jogo e carregar estados antigos(load).
    Nela estão guardados todos os dados relativos a uma partida do jogo*/
    char mapa[MAXALTURA*2][MAXLARGURA*2]; // matriz do mapa parcial
    char nivel[11]; // nome do arquivo de nivel a ser carregado
    char nome_jogador[12]; //nome do jogador ate 14 caracteres
    char nome_arquivo[15]; //nome do arquivo contendo os dados da estrutura save
    char style_icon; // estilo visual do avatar
    int coef_move; //coeficiente da funcao move, serve para indicar ao laco de jogo na main() se o jogo deve ir ou nao para a fase seguinte
    float tempo;
    float tempo_total; // tempo restante
    float pontuacao; //pontuacao total
    int vidas;
    char senha[6]; //Senha para acessar o load game. Lembrar de restringir para 5 digitos
    int num_movimentos;
    int total_movimentos; // numero de movimentos ja feitos
    int niveis_completos; //total de niveis completos
    int clock; // [BONUS] numero de ajustes que o jogador pode fazer no relogio
    int game_status; //indicador para informar se a estrutura esta sendo tratada dentro ou fora do jogo
    int load_indicator; // indicador de load. Caso seja 0, jogador nao pode carregar o jogo. Caso seja 1, pode, e caso seja 2, o jogador ja venceu o jogo e pode reacessar sua cont
    int x, y;//coordenadas do jogador
    int caixas, fixas, indice_fixa[5]; // numero de caixas moveis, numero de caixas fixas, indicador para qual caixa ja foi fixada
    int box_Spot[5][2]; // coordenadas das caixas
    int indicador;  //indicador para apontar se a estrutura foi carregada corretamente
    int erro_fase
} Save;

/* funcoes relativas as artes e impressoes do jogo*/
char tela_inicial(int);
void bordas();
void logo();
void game_over_art();
void good_game_art();
void menu_direita(Save);

/*funcoes relativas ao campo de jogo: abertura e expansao dos mapas, gerador de espacos aleatorios e impressao do campo*/
Save imprime_Campo(Save);
Save aumenta_matriz(char original[MAXALTURA][MAXLARGURA],char aumentado[MAXALTURA*2][MAXLARGURA*2], Save);
Save gera_Spots(Save);
Save imprime_matriz(char aumentado[MAXALTURA*2][MAXLARGURA*2], Save);

/*funcoes relativas as opcoes do menu inicial*/
Save novo_jogo();
Save load();
Save highscore(Save);
void top10(int);
Save tutorial(Save);

/*funcoes relativas aos saves de dados dos jogadores e dos highscores */
void save_level(Save);
void save_highscore(Highscore);
Highscore regula_score(Save);
Save regula_save(Save);

/*funcoes relativas ao que se passa dentro do jogo: testes, movimentacao, condicoes de vitoria, etc*/
int menu_Superior(int);
Save jogo_principal(Save*, Highscore*, int, int);
void movimentacao(Save, int*, int*, int*, int*, int*, int*, float*, int, int, int, int, int, int, int, int);
int checagem(char buffer[8]);
void Menu(int, float, int, int, int, int);
void move_Jogador(int, int, int, int, char);
void pause_game();

int main ()
{
    Highscore scoreSt;
    Save saveSt;
    char option;
    int chamadas_Fase;
    int quit = 0;
    int check_animation;// caso 0, printa animacao do menu inicial surgindo, cc nao mostra a animacao
    saveSt.coef_move = 0; // retorno da funcao de movimentacao
    srand(time(NULL));

    bordas(); //imprime as bordas da area de jogo
    logo();   //imprime o logo do jogo "Sadness Sokoban"

    do    // laco de retorno para a tela principal atraves da funcao de movimentacao (tela de jogo)
    {
        check_animation = 0;
        saveSt.indicador = 0; //reinicializa indicador de referencia para permanencia no menu
        saveSt.coef_move = 0;  // reinicializacao do coef_Movimento
        saveSt.game_status = 0;

        do
        {
            chamadas_Fase = 0; // reinicializacao do contador de fases
            clrscr();

            //laco de retorno ao menu principal atraves de alguma das opcoes do proprio menu, caso alguma opcao nao seja efetivada corretamente
            do //laco de checagem, para que apenas os valores corretos de opcao sejam aceitos
            {
                bordas();
                option = tela_inicial(check_animation); //imprime tela inicial e atribui a opcao escolhida pelo jogador em option
                check_animation = 1;

            }
            while (option != 'n' && option != 'c' && option != 'e' && option != 't' && option != 's'); //aceita apenas valores validos p/ sair do laco
            clrscr();
            switch(option) // casos para cada opcao
            {
            case'n': // caso o jogador escolha Novo Jogo
                saveSt = novo_jogo(); //abre tela de entrada para novo jogo, e salva algumas variaveis na estrutura saveSt
                break;
            case'c': // caso o jogador escolha carregar um jogo já criado
                saveSt = load(); //abre tela para busca de jogador/jogo, e atribui os valores na estrutura já criada para a estrutura saveSt
                chamadas_Fase = saveSt.niveis_completos;
                break;
            case 'e': // caso o jogador escolha a tela de escores
                saveSt = highscore(saveSt); //abre tela com os melhores escores para que o jogador possa ver
                break;
            case 't':
                saveSt = tutorial(saveSt);/*abre a tela de tutorial, que possui instruções sobre o jogo*/
                break;
            case 's':
                saveSt.indicador = 1;/*comando para sair do jogo*/
                quit = 1;
                break;

            }
        }
        while(saveSt.indicador == 0); //caso a variavel saveSt.indicador seja 0, continua no laco. Serve para que o jogador volte ao menu em casos especificos como {nao foi possivel carregar o jogador} ou {visualizacao de highscore/tutorial}

        clrscr();
        if(!quit )// caso o jogador tenha escolhido sair do jogo atraves do menu principal, nao entra no laco de jogo
            do
            {
                clrscr();
                switch(chamadas_Fase)
                {
                case 1:
                    saveSt.nivel[5] = '2'; //muda o nivel para 2
                    break;
                case 2:
                    saveSt.nivel[5] = '3'; // muda o nivel para 3
                    break;
                case 3:
                    saveSt.nivel[5] = '4'; // muda o nivel para 4
                    break;
                }
                saveSt = imprime_Campo(saveSt);// retorna coordenados xy da pos. inicial do jogador, e o numero de caixas na fase
                saveSt = jogo_principal(&saveSt, &scoreSt, (saveSt.x)-1, saveSt.y); //retorna o coef_Movimento para que o programa siga a direcao correta de execucao
                if(saveSt.coef_move != ESC)
                    chamadas_Fase++;

            }
            while( chamadas_Fase != 4 && saveSt.coef_move != ESC && saveSt.coef_move != 'N' && saveSt.coef_move != 'E' && saveSt.coef_move != 'L');
    }
    while(saveSt.coef_move == 'N');  // repete o laco caso, no menu de jogo, seja escolhido Novo jogo
    return 0;
}

/*funcoes relativas a artes e impressoes do jogo*/

char tela_inicial(int check) // check informa a funcao se a animacao de tela inicial deve ou nao ser feita, a função retorna para a main qual opção o jogador escolheu
{
    int i, j;
    char eegg[25] = {"por G.Malta & C.Morvan"};
    char versao[20] = {"V. 1.0"};
    char sigla[7][10] = {{"##########"},{"#        #"},{"#         "},{"##########"},{"         #"},{"#        #"},{"##########"}};
    char brasao[3][25] = { {"#########################"}, {"# # # # # # # # # # # # #"}, {"#########################"} };
    char option;


    textcolor(WHITE);
    textbackground(WHITE);
    for(i=4; i<22; i++)
    {
        gotoxy(21, i);
        printf("||");
        gotoxy(48, i);
        printf("||");
    }

    for(i=0; i<3; i++)
    {
        for(j=0; j<25; j++)
        {
            if(brasao[i][j] == '#')
            {
                textbackground(RED);
                textcolor(RED);
            }
            gotoxy(23+j, 6+i);
            printf("%c", brasao[i][j]);
            textbackground(BLACK);
        }

    }
    textbackground(BLACK);
    for(i=0; i<7; i++)
    {
        for(j=0; j<10; j++)
        {
            if(sigla[i][j] == '#')
            {
                textbackground(LIGHTMAGENTA);
                textcolor(LIGHTMAGENTA);
            }

            gotoxy(6+j, 10+i);
            printf("%c", sigla[i][j]);
            gotoxy(54+j, 10+i);
            printf("%c", sigla[i][j]);
            textbackground(BLACK);
        }
        if(check == 0)
            delay(10);
    }
    textcolor(WHITE);
    textbackground(BLACK);
    gotoxy(27, 11);
    printf("[N]Novo jogo");
    gotoxy(27, 12);
    printf("[C]Carregar jogo");
    gotoxy(27, 13);
    printf("[E]Escores");
    gotoxy(27, 14);
    printf("[T]Tutorial");
    gotoxy(27, 15);
    printf("[S]Sair");

    for(i=0; i<strlen(versao); i++)
    {
        textcolor(LIGHTCYAN);
        gotoxy(58+i, 21);
        printf("%c", versao[i]);
        if(check == 0)
            delay(125);
        textcolor(WHITE);
    }


    while(!kbhit())
    {
        for(i=0; i<15; i++)
        {
            if(kbhit())
                i=15;
            if(i<=6)
            {
                textcolor(BROWN);
                gotoxy(36, 19);
                printf("XX");
                gotoxy(36, 20);
                printf("XX");
            }
            textcolor(LIGHTMAGENTA);
            gotoxy(27+i, 19);
            printf("++");
            gotoxy(27+i, 20);
            printf("++");
            if(i<=6)
                delay(500);
            if(i>6)
            {
                textcolor(BROWN);
                gotoxy(27+2+i, 19);
                printf("XX");
                gotoxy(27+2+i, 20);
                printf("XX");
                delay(500);

            }
            gotoxy(27+i, 19);
            printf("     ");
            gotoxy(27+i, 20);
            printf("     ");
            if(kbhit())
                i=15;
        }

    }
    textcolor(WHITE);

    option = getch();

    if(option == 'p'){
        for(i=0; i<strlen(eegg); i++)
    {
        textcolor(LIGHTCYAN);
        gotoxy(24+i, 18);
        printf("%c", eegg[i]);
            delay(125);
    }
        textcolor(WHITE);
    }

    return option;
}

void bordas() //Imprime as bordas dos menus e do mapa.
{
    int i;
    int hor_bar = 20;

    textcolor(WHITE);
    textbackground(WHITE);
    for(i=1; i<(MAXLARGURA*2)+hor_bar+1; i++)
    {
        gotoxy(i, 4);
        printf("0");
        gotoxy(i, 5);
        printf("0");
        gotoxy(i, (MAXALTURA*2)+2);
        printf("0");
        gotoxy(i, (MAXALTURA*2)+3);
        printf("0");
    }
    for(i=0; i<(MAXALTURA*2)+2; i++)
    {
        gotoxy(1, i);
        printf("0");
        gotoxy((MAXLARGURA*2)+ hor_bar-1, i);
        printf("00");
    }
    textbackground(BLACK);
}

void logo() //A função printa o logo do game no inicio da execução do programa
{
    /* /Essa função printa o logo do jogo ao iniciar e espera alguma interação do jogador*/
    int i,j, change;
    char go;
    char logo[11][41]=
    {
        {"##### ##### ###   #   # ##### ##### #####"},
        {"#     #   # #  #  ##  # #     #     #	   "},
        {"##### ##### #   # # # # ####  ##### #####"},
        {"    # #   # #  #  #  ## #         #     #"},
        {"##### #   # ###   #   # ##### ##### #####"},
        {"                                         "},
        {"##### ##### #  ## ##### ####  ##### #   #"},
        {"#     #   # # #   #   # #   # #   # ##  #"},
        {"##### #   # ##    #   # ####  ##### # # #"},
        {"    # #   # # #   #   # #   # #   # #  ##"},
        {"##### ##### #  ## ##### ####  #   # #   #"}
    };
    textcolor(LIGHTMAGENTA);
    gotoxy(12, 8);
    for(i=0; i<11; i++)
    {
        for(j=0; j<41; j++)
        {
            if(logo[i][j] == '#')
                textbackground(LIGHTMAGENTA);
            else textbackground(BLACK);
            printf("%c", logo[i][j]);
        }
        delay(75);
        printf("\n");
        gotoxy(12, 9+i);
    }
    textcolor(YELLOW);
    textbackground(BLACK);
    change = 1;
    fflush(stdin);

    while(!kbhit()) /*Enquanto não houver interação do jogador a mensagem fica "piscando" na tela*/
    {
        gotoxy(25, 20);
        if(change%2)
            printf("PRESS ANY BUTTOM");
        else printf("                ");
        change++;
        delay(500);
    } //hold de 2s na tela de logo
    textcolor(WHITE);

}

void game_over_art() //A função printa a arte de game over quando o jogo termina
{
    char game_over[16][44] =
    {
        {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"},
        {"x                                        x"},
        {"x      000000   000   00    00 000000    x"},
        {"x     0        0   0  0 0  0 0 0         x"},
        {"x    0   0000 0000000 0  00  0 0000      x"},
        {"x     0     0 0     0 0      0 0         x"},
        {"x      000000 0     0 0      0 000000    x"},
        {"x                                        x"},
        {"x     000000  0     0 000000 000000      x"},
        {"x    0      0 0     0 0      00    0     x"},
        {"x    0      0 0     0 0000   000000      x"},
        {"x    0      0  0   0  0      00   00     x"},
        {"x     000000    000   000000 00     0    x"},
        {"x                                        x"},
        {"x                                        x"},
        {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}
    };
    int i, j;
    for(i=0; i<16; i++)
    {
        for (j=0; j< 44; j++)
        {
            switch(game_over[i][j])
            {
            case ' ':
                textbackground(BLACK);
                break;
            case 'x':
                textcolor(WHITE);
                textbackground(WHITE);
                break;
            case '0':
                textcolor(RED);
                textbackground(RED);
                break;
            }
            gotoxy(3+j, 6+i);
            printf("%c", game_over[i][j]);
        }
    }
    textcolor(WHITE);
    textbackground(BLACK);

}

void good_game_art() //A função printa a arte de good game, exibida quando o jogador conclui o jogoc om exito
{
    char good_game[16][44] =
    {
        {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"},
        {"x                                        x"},
        {"x      000000  00000   00000  00000      x"},
        {"x     0       0     0 0     0 0    0     x"},
        {"x    0   0000 0     0 0     0 0     0    x"},
        {"x     0     0 0     0 0     0 0    0     x"},
        {"x      000000  00000   00000  00000      x"},
        {"x                                        x"},
        {"x      000000   000   00    00 000000    x"},
        {"x     0        0   0  0 0  0 0 0         x"},
        {"x    0   0000 0000000 0  00  0 0000      x"},
        {"x     0     0 0     0 0      0 0         x"},
        {"x      000000 0     0 0      0 000000    x"},
        {"x                                        x"},
        {"x                                        x"},
        {"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"}
    };
    int i, j;
    for(i=0; i<16; i++)
    {
        for (j=0; j< 44; j++)
        {
            switch(good_game[i][j])
            {
            case ' ':
                textbackground(BLACK);
                break;
            case 'x':
                textcolor(WHITE);
                textbackground(WHITE);
                break;
            case '0':
                textcolor(YELLOW);
                textbackground(YELLOW);
                break;
            }
            gotoxy(3+j, 6+i);
            printf("%c", good_game[i][j]);
        }
    }
    textcolor(WHITE);
    textbackground(BLACK);

}

void menu_direita(Save save) //Imprime o esqueleto dos menus colocando as informações fixas em seus devidos lugares
{
    int i, j;
    int hor_bar = 30;

    textcolor(WHITE);
    if(save.game_status)   //caso true, indica que o highscore foi acessado de dentro do jogo
    {
        for(i=0; i<16; i++)
        {
            for(j=0; j<23; j++)
            {
                gotoxy(51+j, 6+i);
                printf(" ");
            }
        }
        gotoxy(56, 6);
    }
    textbackground(WHITE);

    for(i=0; i<hor_bar; i++)
    {
        gotoxy((MAXLARGURA*2)+i-1, 4);
        printf("0");
        gotoxy((MAXLARGURA*2)+i-1, 5);
        printf("0");
        gotoxy((MAXLARGURA*2)+i-1, (MAXALTURA*2)+2);
        printf("0");
        gotoxy((MAXLARGURA*2)+i-1, (MAXALTURA*2)+3);
        printf("0");
    }
    for(i=0; i<(MAXALTURA*2)+4; i++)
    {
        gotoxy((MAXLARGURA*2)+ hor_bar-1, i);
        printf("00");
    }
    textbackground(BLACK);
    menu_Superior(0);
    gotoxy(52, 7);
    textcolor(YELLOW);
    printf("Nivel %c", save.nivel[5]);
    gotoxy(53, 9);
    textcolor(WHITE);
    puts(save.nome_jogador);
    gotoxy(52, 12);
    textcolor(YELLOW);
    printf("Ajustes");
    gotoxy(52, 15);
    printf("Tempo");
    gotoxy(60, 15);
    printf("Caixas");
    gotoxy(69, 15);
    printf("Fixas");
    gotoxy(67, 19);
    printf("Movimentos");
    textcolor(LIGHTMAGENTA);
    gotoxy(55, 19);
    printf("Vidas");
    textcolor(WHITE);
}

/*funcoes relativas ao campo de jogo: abertura e expansao dos mapas, gerador de espacos aleatorios e impressao do campo*/

Save imprime_matriz(char aumentado[MAXALTURA*2][MAXLARGURA*2], Save save) //imprime a matriz do campo e passa coords do ponto inicial do jogador
{
    int i,j,k;
    save.caixas = 0;
    k = 0;
    menu_direita(save);

    gotoxy(1, 4);//deixa espaco superior para menu
    for(i=0; i<(MAXALTURA*2); i++)
    {
        for(j=0; j<(MAXLARGURA*2); j++)
        {

            switch (aumentado[i][j])
            {
            case 'x':
                textcolor(WHITE);
                textbackground(WHITE);
                aumentado[i][j] = '0';
                break;
            case '0':
                aumentado[i][j] = ' ';
                break;
            case '3':
                aumentado[i][j] = ' ';
                if((aumentado[i][j-1] == ' ' || aumentado[i][j-1] == '0') && aumentado[i+1][j] == '3' && aumentado[i+1][j+1] == '3')
                {
                    save.box_Spot[k][0] = j+1;
                    save.box_Spot[k][1] = i+4;
                    k++;
                }
                break;
            case '2':
                textcolor(BROWN);
                aumentado[i][j] = 'X';
                save.caixas++;
                break;
            case '1':
                textcolor(LIGHTMAGENTA);
                aumentado[i][j] = save.style_icon;
                save.x = j+1;
                save.y = i+3;
                break;
            }
            printf("%c",aumentado[i][j]);
            textbackground(BLACK);

        }

    }
    switch(save.nivel[5])//Switch para selecionar quanto tempo o jogador terá para resolver a fase atual
    {
    case '1':
        save.tempo = 40;
        break;
    case '2':
        save.tempo = 30;
        break;
    case '3':
        save.tempo = 20;
        break;
    case '4':
        save.tempo = 8;
        break;
    }
    save.caixas = save.caixas/4;
    return save;
}

Save aumenta_matriz(char original[MAXALTURA][MAXLARGURA],char aumentado[MAXALTURA*2][MAXLARGURA*2], Save save) //função recebe o .txt do mapa e retorna a matriz expandida
{
    int i,j;
    int k=0,l=0;
    for(i=0; i<MAXALTURA; i++)
    {
        l=0;
        for(j=0; j<MAXLARGURA; j++)
        {
            if(original[i][j]=='0')
            {
                aumentado[k][l]=' ';
                aumentado[k+1][l]=' ';
                aumentado[k][l+1]=' ';
                aumentado[k+1][l+1]=' ';
                save.mapa[k][l]=' ';
                save.mapa[k+1][l]=' ';
                save.mapa[k][l+1]=' ';
                save.mapa[k+1][l+1]=' ';
            }
            else if(original[i][j] == '\n')
            {
                aumentado[k][l]= ' ';
                aumentado[k][l+1] = '\n';
                aumentado[k+1][l]= ' ';
                aumentado[k+1][l+1] = '\n';
                save.mapa[k][l]= ' ';
                save.mapa[k][l+1] = '\n';
                save.mapa[k+1][l]= ' ';
                save.mapa[k+1][l+1] = '\n';
            }
            else
            {
                aumentado[k][l]=original[i][j];
                aumentado[k+1][l]=original[i][j];
                aumentado[k][l+1]=original[i][j];
                aumentado[k+1][l+1]=original[i][j];
                save.mapa[k][l]=original[i][j];
                save.mapa[k+1][l]=original[i][j];
                save.mapa[k][l+1]=original[i][j];
                save.mapa[k+1][l+1]=original[i][j];
            }
            l+=2;
        }
        k+=2;
    }
    return save;
}

Save gera_Spots(Save save) //Função recebe o save do jogador e retorna quais são os spots onde as caixas devem ser colocadas
{
    int lin, col;
    int contador=0;
    int i, j;
    int deny = 0;
    for(i=0; i<2; i++)
    {
        for(j=0; j<5; j++)
            save.box_Spot[j][i] = 0;
    }

    while(contador < save.caixas)//Enquanto o npumero de caixas do mapa não for exedido o loop deve ser repetido.A função também checa se ja existe caixa na mesma posição.
    {
        deny = 0;
        lin = (rand()%(MAXALTURA*2));
        col = (rand()%(MAXLARGURA*2));
        if(save.mapa[lin][col] == ' ' && lin%2 == 0 && col % 2 == 1)
        {
            for(i=0; i<5; i++)
            {
                if(save.box_Spot[i][0] == col && save.box_Spot[i][1] == lin+4)
                {
                    deny = 1;
                }
            }
            if(!deny)
            {
                save.box_Spot[contador][0] = col;
                save.box_Spot[contador][1] = lin + 4;
                contador++;
            }

        }
    }


    return save;
}

Save imprime_Campo(Save save) //Essa função coordena a abertura, expansão e impressão do mapa
{
    FILE *arq;
    char original[MAXALTURA][MAXLARGURA];
    char aumentada[MAXALTURA*2][MAXLARGURA*2];


    if(!(arq = fopen(save.nivel,"r")))
    {
        printf("Erro na abertura da fase");
        save.indicador = 0;
    }
    else
    {
        if( 250 != (fread(&original,sizeof(char),250,arq)) )
            printf("Erro");
        save = aumenta_matriz(original, aumentada, save);
        save = imprime_matriz(aumentada, save);
        if(save.nivel[5] != '4')
        {
            save = gera_Spots(save);
        }

        fclose(arq);
    }
    return save;
}

/*Funcoes relativas ao menu principal*/

Save novo_jogo() // Função que inicializa um novo jogo, criando um novo save para o jogador
{
    FILE *arq;
    Save save;
    int check;
    char navega;
    int opcao = 0; //opcao pelo visual do personagem
    int cont_navega = 1; //contador de navegacao pelo menu de estilo do personagem

    bordas(); //imprime bordas da tela de jogo
    gotoxy(9,8);
    strcpy(save.nivel, "nivel1.txt");
    printf("Nome[ate 10 caract]: ");
    textcolor(LIGHTCYAN);
    fflush(stdin);
    fgets(save.nome_jogador, MAXNOME-5, stdin);
    textcolor(WHITE);
    save.nome_jogador[strlen(save.nome_jogador) - 1] = '\0';
    strcpy(save.nome_arquivo, save.nome_jogador);
    strcat(save.nome_arquivo, ".bin");//Cria o nome do arquivo de save do jogador



    if(!(arq = fopen(save.nome_arquivo, "rb")))//Teste para ver se o jogador ja possui um save
    {
        check = 0;
    }
    else
    {
        check = 1;
    }
    fclose(arq);


    //busca_jogador

    if(!check) //Caso o nome do jogador não tenha sido encontrado
    {

        gotoxy(9,10);
        printf("Crie uma senha para sua memoria");
        textcolor(YELLOW);
        printf("[ate 5 digitos]:");
        gotoxy(15, 11);
        printf("-> ");
        textcolor(LIGHTCYAN);
        fflush(stdin);
        fgets(save.senha, 5, stdin);
        textcolor(WHITE);
        save.senha[strlen(save.senha) - 1] = '\0';//Salva a senha que o jogador inseriu, esta será usada mais tarde para carregar o save.

        save.total_movimentos = 0;//Reseta os valores do save, definindo todos para os valores iniciais do jogo.
        save.tempo_total = 0;
        save.niveis_completos = 0;
        save.clock = 3;
        save.vidas = 2;
        save.indicador = 1;
        gotoxy(9, 13);
        printf("Escolha o estilo do seu personagem:");//Aqui são exibidos na tela diversos modelos de jogador
        textcolor(LIGHTMAGENTA);
        gotoxy(22, 15);
        printf("%c%c", '+','+');
        gotoxy(22, 16);
        printf("%c%c", '+','+');
        gotoxy(28, 15);
        printf("%c%c", 254, 254 );
        gotoxy(28, 16);
        printf("%c%c", 254, 254 );
        gotoxy(34, 15);
        printf("%c%c", 178, 178 );
        gotoxy(34, 16);
        printf("%c%c", 178, 178 );
        gotoxy(40, 15);
        printf("%c%c", 206, 206 );
        gotoxy(40, 16);
        printf("%c%c", 206, 206 );
        gotoxy(46, 15);
        printf("%c%c", 176, 176 );
        gotoxy(46, 16);
        printf("%c%c", 176, 176 );
        textcolor(YELLOW);
        gotoxy(22, 17);
        printf("==");
        do
        {
            navega = getch();
            if(navega == DIR || navega == ESQ) //Neste trecho, o jogador navega entre as opçoes de personagem, escolhendo com enter seu modelo favorito
            {
                gotoxy(22+((cont_navega-1)*6), 17);
                printf("  ");
                switch (navega)
                {
                case DIR:
                    if(cont_navega < 5)
                        cont_navega++;
                    break;
                case ESQ:
                    if(cont_navega > 1)
                        cont_navega--;
                    break;
                }
                gotoxy(22+((cont_navega-1)*6), 17);
                printf("==");
            }
            else if(navega == ENTER)
            {
                opcao = cont_navega;
            }
        }
        while(opcao < 1 || opcao > 5);

        textcolor(WHITE);
        switch(opcao) //switch utilizado para salvar o modelo do personagem, este é baseado no numero de vezes em que o jogador moveu o seletor antes de escolher seu modelo
        {
        case 1:
            save.style_icon = '+';
            break;
        case 2:
            save.style_icon = 254;
            break;
        case 3:
            save.style_icon = 178;
            break;
        case 4:
            save.style_icon = 206;
            break;
        case 5:
            save.style_icon = 176;
            break;
        }

    }
    else
    {
        gotoxy(5,10);//Caso onde a busca do jogador retorna a existencia de um save com mesmo nome, neste caso o jogo retorna ao menu e o jogador deve iniciar um novo jogo valido.
        printf("Desculpe, ja existe um jogador com esse nome.");
        delay(2000);
        save.indicador = 0;
    }

    return save;
}

Save load() //Função que busca um jogador e retorna seu save no caso do mesmo existir.
{
    FILE *arq;
    Save save_busca;
    int i, j;
    char senha[11];
    int leave = 0;

    clrscr();
    bordas(); //imprime bordas da tela de jogo

    gotoxy(25,11);

    printf("Nome do jogador: ");//Aqui o jogador insere seu nome, para que seja realizada a busca de seu save
    fflush(stdin);
    textcolor(LIGHTCYAN);
    fgets(save_busca.nome_jogador, MAXNOME-5,stdin);
    textcolor(WHITE);
    save_busca.nome_jogador[strlen(save_busca.nome_jogador) - 1]= '\0';

    strncpy(save_busca.nome_arquivo, save_busca.nome_jogador, MAXNOME-7);
    strncat(save_busca.nome_arquivo, ".bin", 5);


    if(!(arq = fopen(save_busca.nome_arquivo, "r")))//O pograma abre o save, e no caso de falha exibe uma mensagem de erro e retorna pro menu
    {
        leave = 1;
        gotoxy(25,13);
        textcolor(RED);
        printf("Jogador nao encontrado.");
        textcolor(WHITE);
        save_busca.indicador = 0;
        save_busca.game_status = 0;
        delay(2500);
    }
    else
    {
        if(fread(&save_busca, sizeof(save_busca), 1, arq) == 1) //Caso exista o jogador, grava no end de save_busca a estrutura do jogador
        {
        }
    }
    fclose(arq);
    if( save_busca.load_indicator != 0 && leave == 0)  //caso indicador = 0, o jogo  nao pode ser carregadom pois o jogador ja perdeu o jogo. Caso seja 2, o jogador ja zerou o jogo e pode reacessar sua conta
    {
        gotoxy(25, 12);
        printf("Senha [ate 10 caracteres]: ");
        fflush(stdin);
        textcolor(LIGHTCYAN);
        fgets(senha, 10, stdin);
        textcolor(WHITE);
        senha[strlen(senha) - 1] = '\0';
        if( !(strncmp(save_busca.senha, senha, strlen(save_busca.senha))))
        {
            save_busca.indicador = 1;
            if(save_busca.load_indicator == 2 || save_busca.load_indicator == 3)
            {
                save_busca.load_indicator = 3;
                gotoxy(7, 13);
                printf("Parabens, %s! Vejo que voce ja finalizou SaS!", save_busca.nome_jogador);//No caso do jogador ja ter concluido o jogo ele pode escolher uma fase para jogar novamente.
                gotoxy(5, 14);
                printf("Em qual nivel deseja retornar para reviver sua experiencia?");
                do
                {
                    gotoxy(15, 15);
                    printf("\t\t-> ");
                    textcolor(LIGHTCYAN);
                    scanf(" %c", &save_busca.nivel[5]);
                    textcolor(WHITE);
                }
                while(save_busca.nivel[5] != '1' && save_busca.nivel[5] != '2' && save_busca.nivel[5] != '3' && save_busca.nivel[5] != '4');

            }
        }
        else  //Caso onde o jogador inseriu a senha incorretamente
        {
            gotoxy(25, 13);
            textcolor(RED);
            printf("<Desculpe, a senha nao corresponde.>");
            textcolor(WHITE);
            delay(2500);
            save_busca.game_status = 0;
            save_busca.indicador = 0;
        }
    }
    else if(leave == 0)  //Caso onde o jogador ja perdeu o jogo e seu save não pode ser carregado.
    {
        gotoxy(19, 13);
        textcolor(RED);
        printf("Sinto muito, este jogador nao pode ser carregado.");
        textcolor(WHITE);
        delay(1000);
        save_busca.indicador = 0;
        save_busca.coef_move = 0;
        save_busca.game_status = 0;
    }

    return save_busca;
}

void top10(int game_stat)   //game_stat e o indicador para informar se o highscore esta sendo aberto na tela inicial ou dentro do jogo
{


    FILE *arq;
    Highscore scores;
    Highscore top10[10];
    int check_nome;
    int stop = 0;
    int i, j;

    for(i=0; i<10; i++)/*loop para zerar o vetor com as melhores pontuações*/
    {
        top10[i].pontuacao = 0;
    }

    if(!game_stat)/*game_stat 0, é o quando o jogador requisita o escore pelo menu e 1, quando requisita dentro do jogo*/
    {
        bordas();
    }

    if( !(arq = fopen("scores.bin", "rb")) )
    {
        textcolor(RED);
        if(game_stat)
        {
            gotoxy(51, 7);
        }
        else gotoxy(19, 10);
        printf("Ainda nao ha highscores!");
        textcolor(WHITE);
    }
    else
    {
        for(i=0; i<10; i++) //laco para preencher os 10 espacos do top 10
        {
            check_nome = 0;
            while(!feof(arq))  //le ate o final do arquivo de scores
            {
                if(fread(&scores, sizeof(scores), 1, arq) == 1) //guarda a estrutura de score relativa ao contador de arquivo no endereco da estrutura de scores
                {
                    if(scores.pontuacao > top10[i].pontuacao ) // caso a pontuacao do score lido no arquivo seja maior que a pont. atual do top10[i],
                    {
                        for(j=0; j<10; j++) //primeiramente serao feitos testes com os top10 ja registrados na lista
                        {
                            if( !(strcmp(scores.nome_jogador, top10[j].nome_jogador)) )  //caso o jogador do score lido ja exista na lista:
                            {
                                if(scores.pontuacao > top10[j].pontuacao)
                                {
                                    top10[j].pontuacao = scores.pontuacao;
                                    strcpy(top10[j].nome_jogador, scores.nome_jogador);
                                    rewind(arq);
                                }
                                check_nome = 1;
                            }

                        }
                        if(!(check_nome))/*teste para ver se o score do jogador ja esta no highscore*/
                        {
                            top10[i].pontuacao = scores.pontuacao;
                            strcpy(top10[i].nome_jogador, scores.nome_jogador);
                        }
                        check_nome = 0;
                    }
                }
            }
            rewind(arq);
        }

        fclose(arq);
    }
    for(i=0; i<10; i++)/*Loop para a impressão das informações na tela*/
    {
        if(top10[i].pontuacao == 0)
            stop = 1;
        if(!stop)
        {
            if(game_stat)/*No caso do escore ser requisitado dentro da partida o cursor é movido para o menu da direita*/
                gotoxy(52, 7+i);
            else gotoxy(24, 9+i);
            switch (i)
            {
            case 0:
                textcolor(BLACK);
                textbackground(YELLOW);
                break;
            case 1:
                textcolor(BLACK);
                textbackground(LIGHTGRAY);
                break;
            case 4:
                textbackground(BLACK);
                textcolor(WHITE);
                break;
            }
            printf("%-10s\t%.2f\n", top10[i].nome_jogador, top10[i].pontuacao);
            delay(100);

        }
    }
    textcolor(WHITE);
    textbackground(BLACK);

}

Save highscore(Save save)
{
    int i, j;
    char hold_press;
    if(!save.game_status)
        clrscr();
    if(save.game_status)   //caso true, indica que o highscore foi acessado de dentro do jogo
    {
        for(i=0; i<16; i++)
        {
            for(j=0; j<26; j++)
            {
                gotoxy(51+j, 6+i);
                printf(" ");
            }
        }
        gotoxy(56, 6);
    }
    else    // caso contrario, indica que o highscore foi acessado pelo menu principal
    {
        bordas();
        gotoxy(22, 7);

    }
    printf("\tHighscores\n");

    top10(save.game_status);

    save.indicador = 0;
    if(!save.game_status)
    {
        while( !kbhit() )
        {
        }
        hold_press = getch();
        fflush(stdin);
    }


    return save;
}

Save tutorial(Save save) //A função exibe um tutorial com os principais comandos do jogo
{
    /*Nesta funçãoa  tela de tutorial é printada, esta apresenta, de forma resumida, os comando do jogo e o objetivo*/
    clrscr();
    bordas();
    gotoxy(2, 7);
    printf("    Para vencer SaS, basta mover as caixas para as posicoes certas,");
    gotoxy(2, 8);
    printf("   antes que o seu tempo acabe... Boa sorte!");
    textcolor(LIGHTMAGENTA);
    gotoxy(2, 10);
    printf("   [Sistema de pontuacao]");
    textcolor(WHITE);
    gotoxy(2, 11);
    printf("     Para maximizar a pontuacao, o jogador deve encontrar os");
    gotoxy(2, 12);
    printf("   lugares adequados para as caixas no menor tempo possivel,");
    gotoxy(2, 13);
    printf("   e fazendo o menor numero de movimentos.");
    textcolor(LIGHTMAGENTA);
    gotoxy(2, 15);
    printf("   [Comandos in-game]");
    gotoxy(2, 16);
    textcolor(YELLOW);
    printf("   [Movimentacao]: ");
    textcolor(WHITE);
    printf("via setas direcionais");
    gotoxy(2, 17);
    textcolor(YELLOW);
    printf("   [TAB]: ");
    textcolor(WHITE);
    printf("Menu\t\t");
    textcolor(YELLOW);
    printf("              [ENTER]: ");
    textcolor(WHITE);
    printf("Selecao");
    gotoxy(2, 18);
    textcolor(YELLOW);
    printf("   [D]:");
    textcolor(WHITE);
    printf("Dica de lugares de encaixe");
    textcolor(YELLOW);
    printf("    [S]:");
    textcolor(WHITE);
    printf(" Clock-charge!");
    gotoxy(2, 19);
    textcolor(RED);
    printf("    *cuidado: para cada pedido de dica, voce perde 4 segundos no");
    gotoxy(2, 20);
    printf("     tempo restante do nivel");
    textcolor(WHITE);

    while( !kbhit() ) /*Enquanto o jogador nao apertar nenhuma tecla, a tela de tutorial segue sendo exibida*/
    {

    }

    save.indicador = 0;
    return save;//retorna/utiliza o save, pois precisa de seu indicador para saber para onde voltar depois
}

/*funcoes relativas aos saves de dados dos jogadores e dos highscores */

Highscore regula_score(Save save) //Copia o nome e a pontuação do jogador para a estrutura de highscore
{
    Highscore score;

    strcpy(score.nome_jogador, save.nome_jogador);
    score.pontuacao = save.pontuacao;

    return score;
}

void save_highscore(Highscore score) //Abre o arquivo de highscore e adiciona a pontuação do jogador
{
    FILE *arq;
    //clrscr();

    if( !(arq = fopen("scores.bin", "ab+")) )
        printf("Erro ao abrir arquivo.\n");
    else
    {
        fwrite(&score, sizeof(score), 1, arq);
    }
    fclose(arq);
}

void save_level(Save save) //Salva o progresso do jogador em seu arquivo
{
    FILE *arq;
    int i, j;
    //clrscr();

    if( !(arq = fopen(save.nome_arquivo, "wb")) )
        printf("Erro ao abrir arquivo.\n");
    else
    {
        fwrite(&save, sizeof(save), 1, arq);
    }
    fclose(arq);
}

Save regula_save(Save save) //Atualiza as informações para o calculo da pontuação do jogador
{
    int n, m;
    float t;

    save.tempo_total = save.tempo_total + save.tempo;
    save.total_movimentos = save.total_movimentos + save.num_movimentos;
    save.niveis_completos = save.niveis_completos + 1;

    n = save.niveis_completos;
    m = save.total_movimentos;
    t = save.tempo_total;

    save.pontuacao = (1000 * (n*n*n))/ (m + (t/2));

    return save;
}

/*funcoes relativas ao que se passa dentro do jogo: testes, movimentacao, condicoes de vitoria, etc*/

void Menu(int cont_mov, float contagem, int num_caixas, int num_fixas, int vidas, int charge) //Atualiza as informações do menu da direita
{
    textcolor(WHITE);
    gotoxy(55, 13);
    printf("%d", charge);
    gotoxy(53, 16);
    printf("%3.1f", contagem);
    gotoxy(63, 16);
    printf("%d", num_caixas);
    gotoxy(71, 16);
    printf("%d", num_fixas);
    gotoxy(71, 20);
    printf("%3.3d", cont_mov);
    gotoxy(57, 20);
    printf("%d", vidas);
    textcolor(BROWN);
}

int menu_Superior(int select) //Essa função imprime as barras de seleção, e retorna se deve , ou não, continuar no menu
{
    int x;
    int leave = 0;
    gotoxy(1, 1);
    printf("%68s", " ");
    gotoxy(1, 3);
    printf("%68s", " ");
    if(select != 0)
    {
        switch(select)
        {
        case 1:
            x = 3;
            break;
        case 2:
            x = 20;
            break;
        case 3:
            x = 33;
            break;
        case 4:
            leave = 1;
            break;

        }
        if(leave == 0)
        {
            gotoxy(x, 3);
            textcolor(LIGHTMAGENTA);
            printf("==========");
            gotoxy(x, 1);
            textcolor(LIGHTMAGENTA);
            printf("==========");
            textcolor(WHITE);
        }
    }
    textcolor(YELLOW);
    gotoxy(4, 2);
    printf(" [Menu]");
    gotoxy(20, 2);
    printf(" [Escore]");
    gotoxy(34, 2);
    printf(" [Quit]");
    textcolor(WHITE);
    return leave;
}

void pause_game() //imprime indicador de que o jogo foi pausado
{
    textcolor(LIGHTMAGENTA);
    gotoxy(1, 1);
    printf("=================================================");
    gotoxy(1, 3);
    printf("=================================================");
    textcolor(WHITE);
}

void imprime_jogador(int x, int y, int funcao, int cor, char style) //Imprime o jogador no novo local ou apaga o logal anterior.
{

    switch(funcao)
    {
    case 1:
        textcolor(cor);
        gotoxy(x,y);
        printf("%c", style);
        gotoxy(x+1,y);
        printf("%c", style);
        gotoxy(x,y+1);
        printf("%c", style);
        gotoxy(x+1,y+1);
        printf("%c", style);
        textcolor(BROWN);
        break;
    case 2:
        gotoxy(x,y);
        printf(" ");
        gotoxy(x+1,y);
        printf(" ");
        gotoxy(x,y+1);
        printf(" ");
        gotoxy(x+1,y+1);
        printf(" ");
        break;
    }

}

int checagem(char buffer[8]) //Checa se o conteudo do buffer é bloco fixo ou não(o conteudo do buffer é o bloco na frente do jogador
{
    int i, cont=0;
    int check = 0;

    for(i=0; i<8; i++)
    {
        if(buffer[i] == '0' || buffer[i] == '#')
        {
            check = 1; //colide com uma parede ou caixa fixa
        }
        else if(buffer[i] == 'X')
        {
            check = 2; // colide completamente com caixa, e pode empurra-la

        }
    }
    return check;

}

void movimentacao(Save save, int *h, int *v, int *moviment, int *caixas_moveis, int *caixas_fixas, int *indice_fixa, float *countdown, int x1, int y1, int x2, int y2, int xGEsq,int yGCima, int xGDir, int yGBaixo) //Função que receebe a direção para onde o jogador vai se mover e coordena os testes para ver se o movimento foi valido ou não.
{

    int check = 0;
    int i, k;
    int cont_caixa, cont_fixas, p;
    char buffer[8]= {0};
    int x, y, cont_mov;
    int check_fixo = 0;
    x = *h;
    y = *v;
    cont_fixas = *caixas_fixas;
    cont_caixa = *caixas_moveis;
    cont_mov = *moviment;
    //save.caixas;

    // A seguir, testa se o jogador colidirá com uma parede ou com uma caixa
    gettext(x+xGEsq, y+yGCima, x+xGDir, y+yGBaixo, buffer);
    check = checagem(buffer);
    // A seguir, indica o que deve ser feito para cada caso (0- Movimento livre; 1- Colisão com parede; 2- Colisão com caixa)
    switch (check)
    {
    case 0:
        y = y + yGCima;
        x = x + xGEsq;
        cont_mov++;//Contador de movimentos ++, jogador se moveu
        break;
    case 1:
        break;
    case 2:
        y = y + y1;
        x = x + x1;
        //A seguir, testa se ao ser movida, a caixa colidirá ou não com um bloco de parede, ou com outra caixa
        gettext(x+xGEsq, y+yGCima, x+xGDir, y+yGBaixo, buffer);
        for(i=0; i<8; i++)
            if(buffer[i] == '0' || buffer[i] == 'X' || buffer[i] == '#')
            {
                check = 1;
            }
        //Caso não ocorra uma colisão, move a caixa
        if(check != 1)
        {
            for(p=0; p < save.caixas; p++)
            {
                if(save.box_Spot[p][0] == x+x1  && save.box_Spot[p][1] == y+y1)
                {

                    check_fixo = 1;
                    cont_caixa--;
                    cont_fixas++;
                    *(indice_fixa+p) = 1;
                    switch(save.nivel[5])
                    {
                    case '1':
                        *countdown += 10;
                        break;
                    case '2':
                        *countdown += 7;
                        break;
                    case '3':
                        *countdown += 5;
                        break;
                    case '4':
                        *countdown += 3.5;
                        break;
                    }

                    //gotoxy()
                }
            }
            if(check_fixo == 0)
            {
                gotoxy(x+x1, y+y1);
                printf("X");
                gotoxy(x+x2, y+y1);
                printf("X");
                gotoxy(x+x1, y+y2);
                printf("X");
                gotoxy(x+x2, y+y2);
                printf("X");
            }
            else
            {
                textcolor(LIGHTCYAN);
                gotoxy(x+x1, y+y1);
                printf("#");
                gotoxy(x+x2, y+y1);
                printf("#");
                gotoxy(x+x1, y+y2);
                printf("#");
                gotoxy(x+x2, y+y2);
                printf("#");
            }


            cont_mov++; //Contador de movimentos ++, jogador se moveu
        }
        else
        {
            y = y - y1;
            x = x - x1;
        }
        break;
    }
    check_fixo = 0;

    imprime_jogador(x, y, 1, LIGHTMAGENTA, save.style_icon);

    *caixas_moveis = cont_caixa;
    *caixas_fixas = cont_fixas;
    *h = x;
    *v = y;
    *moviment = cont_mov;
}

Save jogo_principal(Save *save, Highscore *score, int x, int y) //Principal função de jogabilidade, ela que coordena e chamas os testes referentes a movimentação e realização de objetivos.
{
    Highscore score_temp;
    Save save_temp;

    char c;
    char end_level = '1';
    int i;
    int num_caixas;
    int cont, k;
    int cont_mov = 0;
    int cont_TAB = 0, option = 0;
    float countdown, tempo_corrido;

    num_caixas = save->caixas;
    score_temp = *score;
    save_temp = *save;
    countdown = save_temp.tempo;
    save_temp.game_status = 1;
    tempo_corrido = 0;
    save_temp.fixas = 0;
    for(i=0; i<save_temp.caixas; i++) //zerar os indices das caixas fixas
        save_temp.indice_fixa[i] = 0;

    textcolor(LIGHTMAGENTA);
    gotoxy(56, 2);
    printf("Sadness Sokoban");

    while(!kbhit())  // laco para que o jogo so comece quando o jogador apertar alguma tecla
    {}
    do
    {
        while(countdown > 0 && !kbhit() )  //laco para contagem regressiva de tempo e impressao no menu, enquanto tempo>0 e enquanto o jogador nao estiver pressionando nenhuma tecla
        {
            delay(50);
            countdown -= 0.06;
            tempo_corrido += 0.06;
            Menu(cont_mov, countdown, save_temp.caixas, save_temp.fixas, save_temp.vidas, save_temp.clock);

        }

        c = getch();
        if(!(strncmp(save_temp.nome_jogador, "admin", 6)))// Apenas para o usuario admin. Possibilita que o admin passe as fases apertando '0' ou saia do jogo apertando ESC
            end_level = c;

        if (c != TAB && c != '0' && countdown > 0 && c != DICA)  // Checa se o usuario nao deseja abrir o menu e se o tempo de jogo ainda nao acabou
        {
            countdown -= 0.0125;
            tempo_corrido += 0.0125;
            switch (c)//Switch para movimentar o jogador para a direção desejada
            {
            case CIMA:
                imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon);
                movimentacao(*save, &x, &y, &cont_mov, &save_temp.caixas, &save_temp.fixas, &save_temp.indice_fixa, &countdown, 0, -2, 1, -1, 0, -2, 1, -2);
                break;

            case BAIXO:
                imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon);
                movimentacao(*save, &x, &y, &cont_mov, &save_temp.caixas, &save_temp.fixas, &save_temp.indice_fixa, &countdown, 0, +2, 1, +3, 0, +2, 1, +2);
                break;

            case DIR:
                imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon);
                movimentacao(*save, &x, &y, &cont_mov, &save_temp.caixas, &save_temp.fixas, &save_temp.indice_fixa, &countdown, 2, 0, 3, 1, 2, 0, 2, 1);
                break;

            case ESQ:
                imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon);
                movimentacao(*save, &x, &y, &cont_mov, &save_temp.caixas, &save_temp.fixas, &save_temp.indice_fixa, &countdown, -2, 0, -1, 1, -2, 0, -2, 1);
                break;
            case CLOCK:
                if(save_temp.clock > 0)
                {
                    save_temp.clock--;
                    imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon); //apaga jogador
                    imprime_jogador(x, y, 1, LIGHTCYAN, save_temp.style_icon); //printa jogador
                    gotoxy(49, 16);
                    textcolor(LIGHTCYAN);
                    printf("-->");
                    delay(2000);
                    imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon); //apaga jogador
                    imprime_jogador(x, y, 1, LIGHTMAGENTA, save_temp.style_icon); //printa jogador em ciano claro
                    gotoxy(49, 16);
                    printf("   ");
                    countdown =  countdown + (15 - (save_temp.niveis_completos * 2));
                }
            }
            Menu(cont_mov, countdown, save_temp.caixas, save_temp.fixas, save_temp.vidas, save_temp.clock);
            if(save_temp.caixas == 0) // caso o contador de caixas restantes chegue a zero, significa que a fase terminou
            {
                end_level = '0';      // end_level = '0' significa que o jogador concluiu a fase atual
                textcolor(LIGHTCYAN);
                gotoxy(x+2, y-1);
                printf("Nice!");
                delay(1000);
                textcolor(WHITE);
                if(save_temp.nivel[5] == '2')
                {
                    save_temp.vidas++;
                    gotoxy(53, 20);
                    textcolor(LIGHTMAGENTA);
                    printf("-->");
                    delay(2000);
                    gotoxy(53, 20);
                    printf("   ");
                    countdown = (save_temp.vidas * 5) + 10;
                    Menu(cont_mov, countdown, save_temp.caixas, save_temp.fixas, save_temp.vidas, save_temp.clock);

                }

            }
        }
        else
        {
            if(countdown > 0)
                switch (c)
                {
                case TAB:
                    pause_game(); // entra no menu superior de opcoes e pausa o jogo
                    do
                    {
                        c = getch();
                        if(c == DIR || c == ESQ || c == TAB)
                        {
                            switch (c)
                            {
                            case DIR:
                                if(cont_TAB < 3)
                                    cont_TAB++;
                                break;
                            case ESQ:
                                if(cont_TAB > 1)
                                    cont_TAB--;
                                break;
                            case TAB:
                                cont_TAB = 4;
                                break;
                            }
                            option = menu_Superior(cont_TAB);
                        }
                        else if(c == ENTER)
                        {
                            switch (cont_TAB)
                            {
                            case 1: //Opcao Novo Jogo do Menu
                                end_level = 'N';    //Caso o jogador escolha Novo jogo, atraves do menu de jogo
                                option = 1;         //para sair (menu in-game)
                                break;
                            case 2: //Opcao Escores do Menu
                                gotoxy(52, 5);
                                save_temp = highscore(save_temp);
                                break;
                            case 3: // Opcao do Menu no QUIT
                                end_level = ESC;   // Caso o jogador escolha Sair, atraves do menu do jogo
                                option = 1;        //para sair do laco micro (menu in-game)
                                break;
                            }
                        }
                    }
                    while(option == 0);
                    cont_TAB = 0;
                    option = 0;
                    menu_direita(save_temp);
                    Menu(cont_mov, countdown, save_temp.caixas, save_temp.fixas, save_temp.vidas, save_temp.clock);
                    break;
                case DICA:
                    imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon); //apaga jogador
                    imprime_jogador(x, y, 1, LIGHTCYAN, save_temp.style_icon); //printa jogador em ciano claro
                    for(k=0; k<save->caixas; k++)
                    {
                        gotoxy(63, 7+k);
                        textcolor(LIGHTCYAN);
                        if(save_temp.indice_fixa[k] == 1)
                            textcolor(RED);
                        printf("[C%d] x:%2.2d y:%2.2d", k+1, save->box_Spot[k][0], save->box_Spot[k][1]); //imprime as Dicas de coordenadas de caixas
                    }
                    delay(2000);
                    imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon); //apaga jogador
                    imprime_jogador(x, y, 1, LIGHTMAGENTA, save_temp.style_icon); //printa jogador em ciano claro
                    for(k=0; k<save->caixas; k++)
                    {
                        gotoxy(63, 7+k);
                        printf("              ");        // apaga as coordenadas dos box_spots
                    }
                    if(countdown >= 0)
                    {
                        countdown -= 4;
                        tempo_corrido += 4;
                    }
                    break;
                }
        }
        if(countdown <= 0)
        {
            save_temp.vidas--;
            if(save_temp.vidas > 0)
            {
                imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon); //apaga jogador
                imprime_jogador(x, y, 1, LIGHTCYAN, save_temp.style_icon); //printa jogador em ciano claro
                gotoxy(53, 20);
                textcolor(LIGHTCYAN);
                printf("-->");
                delay(2000);
                imprime_jogador(x, y, 2, LIGHTMAGENTA, save_temp.style_icon); //apaga jogador
                imprime_jogador(x, y, 1, LIGHTMAGENTA, save_temp.style_icon); //printa jogador em ciano claro
                gotoxy(53, 20);
                printf("   ");
                countdown = (save_temp.vidas * 5) + 10;
                Menu(cont_mov, countdown, save_temp.caixas, save_temp.fixas, save_temp.vidas, save_temp.clock);
            }
            if(save_temp.vidas == 0)
                end_level = 'L';
        }

    }
    while(end_level != '0' && end_level != ESC && end_level != 'N' && end_level != 'L');

    if((end_level == '0' || end_level == ESC || end_level == 'L' || end_level == 'N') && save_temp.load_indicator != 3 ) // '0' indica vitoria no nivel, ESC = sair, 'L' = Derrota. load_indicator == 3 significa que o jogador ja finalizou o jogo e esta jogando novamente
    {

        switch(end_level)
        {
        case '0': //salva no .bin a FASE SEGUINTE, APENAS se o jogador houver concluido a fase atual
            save_temp.load_indicator = 1; //relemebrando, isso significa que o jogador por dar load em sua conta
            switch(save_temp.nivel[5])
            {
            case '1':
                save_temp.nivel[5] = '2';
                save_temp.caixas = 4;
                break;
            case '2':
                save_temp.nivel[5] = '3';
                save_temp.caixas = 5;
                break;
            case '3':
                save_temp.nivel[5] = '4';
                save_temp.caixas = 4;
                break;
            case '4': //Caso o jogador termine o ultimo nivel, e venca SADNESS SOKOBAN
                save_temp.load_indicator = 2;
                good_game_art();
                break;

            }
            break;
        case ESC:  //Caso o jogador saia do jogo, a prox. vez que carregar seu perfil, retornara no inicio da fase
            save_temp.load_indicator = 1;
            break;
        case 'N':
            save_temp.load_indicator = 1;
            break;
        case 'L':// caso o jogador tenha perdido o jogo, nao podera retornar ao jogo novamente
            game_over_art();
            save_temp.load_indicator = 0;
            break;
        }

    }
    if(save_temp.load_indicator != 3 &&  (strncmp(save_temp.nome_jogador, "admin", 6)) && end_level == '0')       //salva o jogo APENAS, se o jogador nao for um usuario que ja finalizou o jogo ou um admin (e caso o usuario tenha vencido o nivel)
    {
        save_temp.num_movimentos = cont_mov;
        save_temp.tempo = tempo_corrido;

        save_temp = regula_save(save_temp);
        score_temp = regula_score(save_temp);

        save_level(save_temp);
        save_highscore(score_temp);
    }
    if (end_level != 'N' &&  (strncmp(save_temp.nome_jogador, "admin", 6)))  // caso o jogador nao escolha "novo jogo", e caso nao seja um admin, printa os highscores
    {
        gotoxy(52, 5);
        textcolor(WHITE);
        save_temp = highscore(save_temp);
        delay(5000);
    }

    save_temp.coef_move = end_level;

    return save_temp; // retorna o coef_movimentacao (na main)
}


