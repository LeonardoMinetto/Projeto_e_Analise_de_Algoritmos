#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>

#define MAXN 10000

int n;
double x[MAXN], y[MAXN];
double reward[MAXN];
double dist[MAXN][MAXN];
bool visitado[MAXN];

int depot = 0;
double custo_max = 0;

double calcular_distancia(int i, int j) {
    double dx = x[i] - x[j];
    double dy = y[i] - y[j];
    return sqrt(dx*dx + dy*dy);
}

void ler_oplib(const char *nome_arquivo) {
    FILE *f = fopen(nome_arquivo, "r");
    if (!f) {
        printf("Erro ao abrir arquivo!\n");
        exit(1);
    }

    char linha[256];

    while (fgets(linha, sizeof(linha), f)) {

        if (strstr(linha, "DIMENSION")) {
            sscanf(linha, "DIMENSION : %d", &n);
        }

        if (strstr(linha, "COST_LIMIT")) {
            sscanf(linha, "COST_LIMIT : %lf", &custo_max);
        }

        if (strstr(linha, "NODE_COORD_SECTION")) {
            for (int i = 0; i < n; i++) {
                int id;
                fscanf(f, "%d %lf %lf", &id, &x[i], &y[i]);
            }
        }

        if (strstr(linha, "NODE_SCORE_SECTION")) {
            for (int i = 0; i < n; i++) {
                int id;
                fscanf(f, "%d %lf", &id, &reward[i]);
            }
        }

        if (strstr(linha, "DEPOT_SECTION")) {
            fscanf(f, "%d", &depot);
            depot= 0; 
        }
    }

    fclose(f);


    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            dist[i][j] = calcular_distancia(i, j);
        }
    }
}

double orienteering_guloso(int inicio, double T) {
    int atual = inicio;
    double tempo_restante = T;
    double recompensa_total = 0;

    for (int i = 0; i < n; i++)
        visitado[i] = false;

    visitado[atual] = true;

    printf("Caminho: %d ", atual);

    while (1) {
        int melhor_no = -1;
        double melhor_score = 0.0;

        for (int i = 0; i < n; i++) {
            if (!visitado[i]) {
                double custo = dist[atual][i];

                if (custo <= 0) continue;

                double score = reward[i] / custo;

                if (custo <= tempo_restante && score > melhor_score) {
                    melhor_score = score;
                    melhor_no = i;
                }
            }
        }

        if (melhor_no == -1)
            break;

        tempo_restante -= dist[atual][melhor_no];
        recompensa_total += reward[melhor_no];
        atual = melhor_no;
        visitado[atual] = true;

        printf("-> %d ", atual);
    }

    printf("\n");

    return recompensa_total;
}

int main() {
    char *arquivos[] = {
        "rat99-gen2-50.oplib",
        "rat195-gen2-50.oplib",
        "rat575-gen2-50.oplib",
        "rat783-gen2-50.oplib",
        "rl1889-gen2-50.oplib",
        "fnl4461-gen3-50.oplib",
        "rl5934-gen2-50.oplib"
        

    };

    int num_instancias = 7;
    double rec[7];
    double rec_otimo[7] = { 2944.0, 5703.0, 17705.0, 24781.0, 60084.0, 164201.0, 176678.0};
    double temp[7]; 
    for (int k = 0; k < num_instancias; k++) {

        printf("\n--------------------------------\n");
        printf("Instância: %s\n", arquivos[k]);
        printf("--------------------------------\n");

        // ler arquivo
        ler_oplib(arquivos[k]);

        printf("Nós: %d\n", n);
        printf("Depot: %d\n", depot);
        printf("Tempo máximo: %.2f\n", custo_max);

        clock_t inicio = clock();

        double resultado = orienteering_guloso(depot, custo_max);
        rec[k] = resultado;
        clock_t fim = clock();

        double tempo_execucao = (double)(fim - inicio) / CLOCKS_PER_SEC;
        temp[k] = tempo_execucao;
        printf("Recompensa total: %.2f\n", resultado);
        printf("Tempo: %f segundos\n", tempo_execucao);
    }
    printf("\n--------------------------------------------------------------------------------------------------------\n");
    printf("|        Instancia          |        Otimo       |      Guloso     |    Qualidade   |     Tempo (s)    |\n");
    for (int k = 0; k < num_instancias; k++) {
        printf("| %-22s    |     %10.1f     |    %10.1f   |   %9.2f%%   |   %12.6f   |\n", arquivos[k], rec_otimo[k], rec[k], (rec[k] / rec_otimo[k]) * 100.0, temp[k]);
    }
    printf("--------------------------------------------------------------------------------------------------------\n");
    return 0;
}