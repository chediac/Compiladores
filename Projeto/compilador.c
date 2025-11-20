/*
Projeto Compiladores 06N - Fase 2
Matheus Chediac Rodrigues       10417490
Lucas Monteiro Soares           10417881
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ID_LEN 15 
#define PRIME_NUMER 211 // Para a Tabela Hash

// ------------------------------------------------------------
// ESTRUTURAS DA TABELA DE SÍMBOLOS E DEFINIÇÕES GLOBAIS
// ------------------------------------------------------------

// Declaração da função hash externa (fornecida pelo professor)
extern int hashMack(char * s);

typedef struct _TNo{
    char ID[16];
    int endereco;
    struct _TNo *prox;
} TNo;

typedef struct {
    TNo *entradas [PRIME_NUMER];
} TTabelaSimbolos;

TTabelaSimbolos tabela_simbolos;
int proximo_endereco_var = 0; // Contador de endereços de variáveis

// Tipos de tokens
typedef enum {
    IDENTIFIER, 
    CONSTINT, 
    CONSTCHAR,

    // Palavras reservadas
    IF, 
    WHILE, 
    PROGRAM, 
    DIV, 
    OR, 
    AND, 
    NOT,
    READ, 
    WRITE, 
    VAR, 
    BEGIN, 
    END, 
    TRUE, 
    FALSE,
    CHAR, 
    INTEGER, 
    BOOLEAN, 
    THEN, 
    ELSE, 
    DO,

    // Símbolos e operadores
    ATRIBUICAO, 
    DOIS_PONTOS, 
    PONTO_VIRGULA, 
    VIRGULA, 
    PONTO,
    MAIS, 
    MENOS, 
    ASTERISCO, 
    ABRE_PAR, 
    FECHA_PAR,
    DIFERENTE, 
    MENOR_IGUAL, 
    MAIOR_IGUAL, 
    IGUAL, 
    MENOR, 
    MAIOR,

    // Utilidades
    COMENTARIO, FIM_ARQUIVO, ERRO
} TAtomo;

// Struct para armazenar info do token
typedef struct {
    TAtomo atomo;
    int linha;
    union {
        int  numero;    
        char id[16];    
        char ch;    
    } atributo;
} TInfoAtomo;

// Variáveis do Scanner 
char *buffer = NULL;
long buf_len = 0;
long pos = 0;
int  numero_linha = 1;
TInfoAtomo tokenAtual;

// ------------------------------------------------------------
// FUNÇÕES DE GERENCIAMENTO DA TABELA DE SÍMBOLOS E ROTULOS
// ------------------------------------------------------------

void inicializar_tabela() {
    for (int i = 0; i < PRIME_NUMER; i++) {
        tabela_simbolos.entradas[i] = NULL;
    }
}

// Insere variável na tabela. Se já existe, erro semântico.
void insere_simbolo(char* id, int linha) {
    int indice = hashMack(id);
    TNo* no_atual = tabela_simbolos.entradas[indice];

    // Verifica duplicidade
    while (no_atual != NULL) {
        if (strcmp(no_atual->ID, id) == 0) {
            fprintf(stderr, "# %d: erro semantico, identificador [%s] ja declarado.\n", linha, id);
            exit(1);
        }
        no_atual = no_atual->prox;
    }

    // Insere novo
    TNo* novo_no = (TNo*) malloc(sizeof(TNo));
    strncpy(novo_no->ID, id, 15);
    novo_no->ID[15] = '\0';
    novo_no->endereco = proximo_endereco_var;
    novo_no->prox = tabela_simbolos.entradas[indice];
    tabela_simbolos.entradas[indice] = novo_no;

    proximo_endereco_var++;
}

// Busca variável. Retorna endereço. Se não existe, erro semântico.
int busca_tabela_simbolos(char* id, int linha) {
    int indice = hashMack(id);
    TNo* no_atual = tabela_simbolos.entradas[indice];

    while (no_atual != NULL) {
        if (strcmp(no_atual->ID, id) == 0) {
            return no_atual->endereco;
        }
        no_atual = no_atual->prox;
    }

    fprintf(stderr, "# %d: erro semantico, identificador [%s] nao declarado.\n", linha, id);
    exit(1);
}

void imprimir_tabela_simbolos() {
    printf("\nTABELA DE SIMBOLOS\n");
    for (int i = 0; i < PRIME_NUMER; i++) {
        TNo* no_atual = tabela_simbolos.entradas[i];
        if (no_atual != NULL) {
            printf("Entrada Tabela Simbolos: [%d]", i);
            while (no_atual != NULL) {
                printf(" => %s | Endereco: %d\n", no_atual->ID, no_atual->endereco);
                no_atual = no_atual->prox;
            }
        }
    }
}

// Gera rótulos únicos (L1, L2, etc.)
int proximo_rotulo() {
    static int rotulo_contador = 1;
    return rotulo_contador++;
}

// ------------------------------------------------------------
// SCANNER (LÉXICO) - MANTIDO ORIGINAL COM PEQUENOS AJUSTES
// ------------------------------------------------------------

void avancar(int n) {
    for (int i = 0; i < n && pos < buf_len; ++i) {
        if (buffer[pos] == '\n') 
        numero_linha++;
        pos++;
    }
}

void erro_lexico(const char *msg) {
    fprintf(stderr, "# %d:erro lexico: %s\n", numero_linha, msg);
    exit(1);
}

TAtomo palavra_reservada(const char *lex) {
    if (strcmp(lex,"if")==0) return IF;
    if (strcmp(lex,"while")==0) return WHILE;
    if (strcmp(lex,"program")==0) return PROGRAM;
    if (strcmp(lex,"div")==0) return DIV;
    if (strcmp(lex,"or")==0) return OR;
    if (strcmp(lex,"and")==0) return AND;
    if (strcmp(lex,"not")==0) return NOT;
    if (strcmp(lex,"read")==0) return READ;
    if (strcmp(lex,"write")==0) return WRITE;
    if (strcmp(lex,"var")==0) return VAR;
    if (strcmp(lex,"begin")==0) return BEGIN;
    if (strcmp(lex,"end")==0) return END;
    if (strcmp(lex,"true")==0) return TRUE;
    if (strcmp(lex,"false")==0) return FALSE;
    if (strcmp(lex,"char")==0) return CHAR;
    if (strcmp(lex,"integer")==0) return INTEGER;
    if (strcmp(lex,"boolean")==0) return BOOLEAN;
    if (strcmp(lex,"then")==0) return THEN;
    if (strcmp(lex,"else")==0) return ELSE;
    if (strcmp(lex,"do")==0) return DO;
    return IDENTIFIER;
}

const char *nome_token(TAtomo a) {
    // (Mantida lista original para erros sintáticos)
    switch (a) {
        case IF: return "if";
        case WHILE: return "while";
        case PROGRAM: return "program";
        case DIV: return "div";
        case OR: return "or";
        case AND: return "and";
        case NOT: return "not";
        case READ: return "read";
        case WRITE: return "write";
        case VAR: return "var";
        case BEGIN: return "begin";
        case END: return "end";
        case TRUE: return "true";
        case FALSE: return "false";
        case CHAR: return "char";
        case INTEGER: return "integer";
        case BOOLEAN: return "boolean";
        case THEN: return "then";
        case ELSE: return "else";
        case DO: return "do";
        case ATRIBUICAO: return "atribuicao";
        case DOIS_PONTOS: return "dois_pontos";
        case PONTO_VIRGULA: return "ponto_virgula";
        case VIRGULA: return "virgula";
        case PONTO: return "ponto";
        case MAIS: return "mais";
        case MENOS: return "menos";
        case ASTERISCO: return "asterisco";
        case ABRE_PAR: return "abre_par";
        case FECHA_PAR: return "fecha_par";
        case DIFERENTE: return "diferente";
        case MENOR_IGUAL: return "menor_igual";
        case MAIOR_IGUAL: return "maior_igual";
        case IGUAL: return "igual";
        case MENOR: return "menor";
        case MAIOR: return "maior";
        case IDENTIFIER: return "identifier";
        case CONSTINT: return "constint";
        case CONSTCHAR: return "constchar";
        case COMENTARIO: return "comentario";
        case FIM_ARQUIVO: return "EOF";
        default: return "ERRO";
    }
}

const char *lexeme_curto(TAtomo a) {
    // (Mantida lista original)
    switch (a) {
        case ATRIBUICAO: return ":=";
        case DOIS_PONTOS: return ":";
        case PONTO_VIRGULA: return ";";
        case VIRGULA: return ",";
        case PONTO: return ".";
        case ABRE_PAR: return "(";
        case FECHA_PAR: return ")";
        case DIFERENTE: return "<>";
        case MENOR_IGUAL: return "<=";
        case MAIOR_IGUAL: return ">=";
        case IGUAL: return "=";
        case MENOR: return "<";
        case MAIOR: return ">";
        case PROGRAM: return "program";
        case VAR: return "var";
        case BEGIN: return "begin";
        case END: return "end";
        case READ: return "read";
        case WRITE: return "write";
        case IF: return "if";
        case THEN: return "then";
        case ELSE: return "else";
        case WHILE: return "while";
        case DO: return "do";
        case OR: return "or";
        case AND: return "and";
        case NOT: return "not";
        case TRUE: return "true";
        case FALSE: return "false";
        case CHAR: return "char";
        case INTEGER: return "integer";
        case BOOLEAN: return "boolean";
        case IDENTIFIER: return "identifier";
        case CONSTINT: return "constint";
        case CONSTCHAR: return "constchar";
        case FIM_ARQUIVO: return "EOF";
        default: return "ERRO";
    }
}

// OBS: No trabalho final, você pode comentar essa função se quiser limpar a saída
void imprimir_token(const TInfoAtomo *t) {
    // Como estamos gerando código MEPA, geralmente não queremos imprimir tokens
    // Descomente abaixo se quiser ver os tokens:
    /*
    if (t->atomo == ERRO || t->atomo == FIM_ARQUIVO) return;
    printf("# %2d:", t->linha);
    if (t->atomo == IDENTIFIER) { printf("identifier : %s\n", t->atributo.id); return; }
    if (t->atomo == CONSTINT) { printf("constint : %d\n", t->atributo.numero); return; }
    if (t->atomo == CONSTCHAR) { printf("constchar : '%c'\n", t->atributo.ch); return; }
    printf("%s\n", nome_token(t->atomo));
    */
}

TInfoAtomo obter_atomo(void) {
    TInfoAtomo t; 
    t.atomo = ERRO; 
    t.linha = numero_linha;

    while (pos < buf_len && isspace((unsigned char)buffer[pos])) 
        avancar(1);

    if (pos >= buf_len || buffer[pos] == '\0') {
        t.atomo = FIM_ARQUIVO;
        t.linha = numero_linha;
        return t;
    }

    if (buffer[pos] == '(') {
        char prox = (pos+1 < buf_len ? buffer[pos+1] : '\0');
        if (prox == '*') {
            int l0 = numero_linha;
            avancar(2);
            while (!(buffer[pos] == '*' && (pos+1 < buf_len && buffer[pos+1] == ')'))) {
                if (pos >= buf_len || buffer[pos] == '\0') 
                erro_lexico("comentario nao terminado");
                avancar(1);
            }
            avancar(2);
            t.atomo = COMENTARIO; 
            t.linha = l0; 
            return t;
        }
    }

    t.linha = numero_linha;

    if (isalpha((unsigned char)buffer[pos]) || buffer[pos] == '_') {
        char lex[MAX_ID_LEN + 1]; 
        int i = 0;
        while (isalnum((unsigned char)buffer[pos]) || buffer[pos] == '_') {
            if (i < MAX_ID_LEN) lex[i++] = buffer[pos];
            else {
                avancar(1);
                while (isalnum((unsigned char)buffer[pos]) || buffer[pos] == '_') 
                avancar(1);
                // Apenas truncar ou erro, vou manter seu erro original ou truncar conforme lógica do amigo
                // Vou manter seu erro para segurança
                erro_lexico("identifier com mais de 15 caracteres");
            }
            avancar(1);
        }
        lex[i] = '\0';
        strncpy(t.atributo.id, lex, MAX_ID_LEN);
        t.atributo.id[MAX_ID_LEN] = '\0';
        t.atomo = palavra_reservada(lex);
        return t;
    }

    if (isdigit((unsigned char)buffer[pos])) {
        long long val = 0;
        while (isdigit((unsigned char)buffer[pos])) {
            val = val*10 + (buffer[pos]-'0');
            avancar(1);
        }
        if (buffer[pos] == 'd') {
            avancar(1);
            if (buffer[pos] == '+') avancar(1);
            if (!isdigit((unsigned char)buffer[pos])) 
                erro_lexico("expoente em constint mal formado");
            int e=0; 
            while (isdigit((unsigned char)buffer[pos])) { 
                e = e*10 + (buffer[pos]-'0'); avancar(1); }
            for (int k=0;k<e;++k) 
                val *= 10;
        }
        t.atomo = CONSTINT; 
        t.atributo.numero = (int)val; 
        return t;
    }

    if (buffer[pos] == '\'') {
        avancar(1);
        if (pos >= buf_len) erro_lexico("constchar mal formado");
        char c = buffer[pos]; 
        avancar(1);
        if (buffer[pos] != '\'') erro_lexico("constchar mal formado");
        avancar(1);
        t.atomo = CONSTCHAR; 
        t.atributo.ch = c; 
        return t;
    }

    switch (buffer[pos]) {
        case ':': if ((pos+1<buf_len)&&buffer[pos+1]=='='){ 
            avancar(2); t.atomo=ATRIBUICAO; } else { avancar(1); t.atomo=DOIS_PONTOS; } return t;
        case ';': avancar(1); t.atomo=PONTO_VIRGULA; return t;
        case ',': avancar(1); t.atomo=VIRGULA; return t;
        case '.': avancar(1); t.atomo=PONTO; return t;
        case '(': avancar(1); t.atomo=ABRE_PAR; return t;
        case ')': avancar(1); t.atomo=FECHA_PAR; return t;
        case '+': avancar(1); t.atomo=MAIS; return t;
        case '-': avancar(1); t.atomo=MENOS; return t;
        case '*': avancar(1); t.atomo=ASTERISCO; return t;
        case '<':
            if ((pos+1<buf_len)&&buffer[pos+1]=='>') { avancar(2); t.atomo=DIFERENTE; return t; }
            if ((pos+1<buf_len)&&buffer[pos+1]=='=') { avancar(2); t.atomo=MENOR_IGUAL; return t; }
            avancar(1); t.atomo=MENOR; return t;
        case '>':
            if ((pos+1<buf_len)&&buffer[pos+1]=='=') { avancar(2); t.atomo=MAIOR_IGUAL; return t; }
            avancar(1); t.atomo=MAIOR; return t;
        case '=': avancar(1); t.atomo=IGUAL; return t;
        default: erro_lexico("simbolo desconhecido");
    }
    return t;
}

void carregar_de_stream(FILE *f) {
    size_t cap = 4096, n = 0;
    buffer = (char*)malloc(cap + 1);
    if (!buffer) { fprintf(stderr, "Erro de alocacao de memoria\n"); exit(1); }
    while (1) {
        size_t r = fread(buffer + n, 1, cap - n, f);
        n += r;
        if (r == 0) break;
        if (n == cap) {
            cap *= 2;
            char *novo = (char*)realloc(buffer, cap + 1);
            if (!novo) { fprintf(stderr, "Erro de alocacao de memoria\n"); free(buffer); exit(1); }
            buffer = novo;
        }
    }
    buffer[n] = '\0';
    buf_len = (long)n;
    pos = 0;
    numero_linha = 1;
}

void carregar_arquivo(const char *nome) {
    FILE *f = fopen(nome, "rb");
    if (!f) { fprintf(stderr, "Erro ao abrir arquivo %s\n", nome); exit(1); }
    carregar_de_stream(f);
    fclose(f);
}

// ------------------------------------------------------------
// SINTÁTICO COM SEMÂNTICA E GERAÇÃO DE CÓDIGO
// ------------------------------------------------------------

void expressao(void); 
void comando(void);     

void proximo() { tokenAtual = obter_atomo(); }

void descarta_comentarios() { while (tokenAtual.atomo == COMENTARIO) { imprimir_token(&tokenAtual); proximo(); } }

void erro_sintatico(TAtomo esp) {
    fprintf(stderr, "# %d:erro sintatico, esperado [%s] encontrado [%s]\n",
            tokenAtual.linha, lexeme_curto(esp), lexeme_curto(tokenAtual.atomo));
    exit(1);
}

void consome(TAtomo esp) {
    descarta_comentarios();
    if (tokenAtual.atomo == esp) {
        imprimir_token(&tokenAtual);
        proximo();
        descarta_comentarios();
    }
    else erro_sintatico(esp);
}

// --- Regras Gramaticais ---

void fator() {
    if (tokenAtual.atomo == IDENTIFIER) {
        // SEMANTICA E CODIGO
        int end = busca_tabela_simbolos(tokenAtual.atributo.id, tokenAtual.linha);
        printf("\tCRVL %d\n", end);
        consome(IDENTIFIER);
    } 
    else if (tokenAtual.atomo == CONSTINT) {
        // CODIGO
        printf("\tCRCT %d\n", tokenAtual.atributo.numero);
        consome(CONSTINT);
    }
    else if (tokenAtual.atomo == CONSTCHAR) {
        // CODIGO (Char tratado como int)
        printf("\tCRCT %d\n", (int)tokenAtual.atributo.ch);
        consome(CONSTCHAR);
    }
    else if (tokenAtual.atomo == ABRE_PAR) {
        consome(ABRE_PAR);
        expressao();
        consome(FECHA_PAR);
    }
    else if (tokenAtual.atomo == NOT) {
        consome(NOT);
        fator();
        printf("\tNEGA\n"); // Negação lógica
    }
    else if (tokenAtual.atomo == TRUE) {
        printf("\tCRCT 1\n");
        consome(TRUE);
    }
    else if (tokenAtual.atomo == FALSE) {
        printf("\tCRCT 0\n");
        consome(FALSE);
    }
    else {
        erro_sintatico(IDENTIFIER);
    }
}

void termo() {
    fator();
    while (tokenAtual.atomo == ASTERISCO || tokenAtual.atomo == DIV) {
        TAtomo op = tokenAtual.atomo; // Guarda operador
        if (tokenAtual.atomo == ASTERISCO) consome(ASTERISCO); else consome(DIV);
        fator();
        
        // GERA CODIGO
        if (op == ASTERISCO) printf("\tMULT\n");
        else printf("\tDIVI\n");
    }
}

void exp_simples() {
    termo();
    while (tokenAtual.atomo == MAIS || tokenAtual.atomo == MENOS) {
        TAtomo op = tokenAtual.atomo; // Guarda operador
        if (tokenAtual.atomo == MAIS) consome(MAIS); 
        else consome(MENOS);
        termo();

        // GERA CODIGO
        if (op == MAIS) printf("\tSOMA\n");
        else printf("\tSUBT\n");
    }
}

void op_rel(TAtomo op) {
    // A função original apenas consumia, agora precisamos gerar código baseada no op
    switch (op) {
        case DIFERENTE: consome(DIFERENTE); break;
        case MENOR: consome(MENOR); break;
        case MENOR_IGUAL: consome(MENOR_IGUAL); break;
        case MAIOR_IGUAL: consome(MAIOR_IGUAL); break;
        case MAIOR: consome(MAIOR); break;
        case IGUAL: consome(IGUAL); break;
        case OR: consome(OR); break;
        case AND: consome(AND); break;
        default: erro_sintatico(IGUAL);
    }
}

void expressao() {
    exp_simples();
    if (tokenAtual.atomo==DIFERENTE||tokenAtual.atomo==MENOR||tokenAtual.atomo==MENOR_IGUAL||
        tokenAtual.atomo==MAIOR||tokenAtual.atomo==MAIOR_IGUAL||tokenAtual.atomo==IGUAL||
        tokenAtual.atomo==OR||tokenAtual.atomo==AND) {
        
        TAtomo op = tokenAtual.atomo; // Salva para gerar código depois
        op_rel(op);
        exp_simples();

        // GERA CODIGO RELACIONAL
        switch (op) {
            case MENOR:        printf("\tCMME\n"); break;
            case MAIOR:        printf("\tCMMA\n"); break;
            case MENOR_IGUAL:  printf("\tCMEG\n"); break;
            case MAIOR_IGUAL:  printf("\tCMAG\n"); break;
            case DIFERENTE:    printf("\tCDES\n"); break; // Ou CMDI, dependendo da MEPA, CDES = Desigualdade
            case IGUAL:        printf("\tCIGU\n"); break;
            case OR:           printf("\tDISJ\n"); break;
            case AND:          printf("\tCONJ\n"); break;
            default: break;
        }
    }
}

void atribuicao() {
    // Precisa pegar o ID para ver onde guardar depois
    char id[16];
    strncpy(id, tokenAtual.atributo.id, 15);
    id[15] = '\0';
    
    // SEMANTICA: Busca endereço
    int end = busca_tabela_simbolos(id, tokenAtual.linha);

    consome(IDENTIFIER);
    consome(ATRIBUICAO);
    expressao();

    // CODIGO
    printf("\tARMZ %d\n", end);
}

void leitura() {
    consome(READ); consome(ABRE_PAR);
    
    // Processa primeiro ID
    char id[16];
    strncpy(id, tokenAtual.atributo.id, 15); id[15] = '\0';
    int end = busca_tabela_simbolos(id, tokenAtual.linha);
    printf("\tLEIT\n");
    printf("\tARMZ %d\n", end);
    consome(IDENTIFIER);

    while (tokenAtual.atomo == VIRGULA) {
        consome(VIRGULA);
        
        strncpy(id, tokenAtual.atributo.id, 15); id[15] = '\0';
        end = busca_tabela_simbolos(id, tokenAtual.linha);
        printf("\tLEIT\n");
        printf("\tARMZ %d\n", end);
        consome(IDENTIFIER);
    }
    consome(FECHA_PAR);
}

void escrita() {
    consome(WRITE); consome(ABRE_PAR);
    
    // Processa primeiro ID
    char id[16];
    strncpy(id, tokenAtual.atributo.id, 15); id[15] = '\0';
    int end = busca_tabela_simbolos(id, tokenAtual.linha);
    printf("\tCRVL %d\n", end);
    printf("\tIMPR\n");
    consome(IDENTIFIER);

    while (tokenAtual.atomo == VIRGULA) {
        consome(VIRGULA);
        
        strncpy(id, tokenAtual.atributo.id, 15); id[15] = '\0';
        end = busca_tabela_simbolos(id, tokenAtual.linha);
        printf("\tCRVL %d\n", end);
        printf("\tIMPR\n");
        consome(IDENTIFIER);
    }
    consome(FECHA_PAR);
}

void condicional() {
    int L1 = proximo_rotulo();
    int L2 = proximo_rotulo();

    consome(IF); 
    expressao(); 
    
    printf("\tDSVF L%d\n", L1); // Se falso, pula pro L1

    consome(THEN); 
    comando();

    if (tokenAtual.atomo == ELSE) {
        printf("\tDSVS L%d\n", L2); // Acabou o THEN, pula o ELSE
        printf("L%d: NADA\n", L1);  // Inicio do ELSE

        consome(ELSE);
        comando();

        printf("L%d: NADA\n", L2); // Fim do IF completo
    } else {
        printf("L%d: NADA\n", L1); // Se não tem else, L1 é o fim
    }
}

void repeticao() {
    int L1 = proximo_rotulo(); // Teste
    int L2 = proximo_rotulo(); // Saida

    printf("L%d: NADA\n", L1);

    consome(WHILE); 
    expressao(); 
    
    printf("\tDSVF L%d\n", L2); // Se falso, sai do loop

    consome(DO); 
    comando();

    printf("\tDSVS L%d\n", L1); // Volta pro teste
    printf("L%d: NADA\n", L2);
}

void comando() {
    switch (tokenAtual.atomo) {
        case IDENTIFIER:
            atribuicao();
            break;
        case READ:
            leitura();
            break;
        case WRITE:
            escrita();
            break;
        case IF:
            condicional();
            break;
        case WHILE:
            repeticao();
            break;
        case BEGIN:
            consome(BEGIN);
            comando();
            while (tokenAtual.atomo == PONTO_VIRGULA) {
                consome(PONTO_VIRGULA);
                comando();
            }
            consome(END);
            break;
        default:
            // Comando vazio é permitido em alguns contextos ou gera erro?
            // No seu código original gerava erro se não fosse Identifier, mas statement pode ser vazio
            // Se não for nenhum dos acima e não for END ou ELSE (que fecham bloco), pode ser erro
            // Vou manter seu padrão:
            // erro_sintatico(IDENTIFIER); 
            // Mas cuidado: se for um comando vazio (ex: ;;), isso pode travar. 
            // Para seguir estritamente seu código original:
             erro_sintatico(IDENTIFIER);
    }
}

void tipo() {
    if (tokenAtual.atomo == CHAR) { consome(CHAR); return; }
    if (tokenAtual.atomo == INTEGER) { consome(INTEGER); return; }
    if (tokenAtual.atomo == BOOLEAN) { consome(BOOLEAN); return; }
    erro_sintatico(CHAR);
}

void declaracao_variaveis() {
    // Captura ID e insere na tabela ANTES de consumir
    char id[16];
    strncpy(id, tokenAtual.atributo.id, 15); id[15] = '\0';
    insere_simbolo(id, tokenAtual.linha);

    consome(IDENTIFIER);
    while (tokenAtual.atomo == VIRGULA) {
        consome(VIRGULA);
        
        // Captura próximos IDs
        strncpy(id, tokenAtual.atributo.id, 15); id[15] = '\0';
        insere_simbolo(id, tokenAtual.linha);
        
        consome(IDENTIFIER);
    }
    consome(DOIS_PONTOS);
    tipo();
}

void bloco() {
    if (tokenAtual.atomo == VAR) {
        consome(VAR);
        declaracao_variaveis(); 
        consome(PONTO_VIRGULA);
        while (tokenAtual.atomo == IDENTIFIER) {
            declaracao_variaveis();
            consome(PONTO_VIRGULA);
        }
        // Fim das declarações: Aloca espaço na pilha
        if (proximo_endereco_var > 0) {
            printf("\tAMEM %d\n", proximo_endereco_var);
        }
    }
    consome(BEGIN);
    comando();
    while (tokenAtual.atomo == PONTO_VIRGULA) {
        consome(PONTO_VIRGULA);
        comando();
    }
    consome(END);
}

void programa() {
    printf("INPP\n"); // Inicia programa MEPA

    consome(PROGRAM);
    consome(IDENTIFIER);
    consome(PONTO_VIRGULA);
    bloco();
    consome(PONTO);
    
    printf("PARA\n"); // Finaliza programa MEPA
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo.pas>\n", argv[0]);
        return 1;
    }

    inicializar_tabela();
    carregar_arquivo(argv[1]);
    tokenAtual = obter_atomo();
    descarta_comentarios();
    
    programa();

    descarta_comentarios();
    if (tokenAtual.atomo != FIM_ARQUIVO) {
        fprintf(stderr, "# %d:erro sintatico, esperado [%s] encontrado [%s]\n",
                tokenAtual.linha, nome_token(FIM_ARQUIVO), nome_token(tokenAtual.atomo));
        return 1;
    }

    // Imprime tabela no final
    imprimir_tabela_simbolos();

    fprintf(stderr, "%d linhas analisadas, programa sintaticamente correto\n", numero_linha);
    return 0;
}