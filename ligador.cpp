#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Remove espaços no começo e no final de uma string
string trim(const string& str) {
  // Encontra o primeiro caractere que não é espaço
  size_t first = str.find_first_not_of(' ');
  // Se não achou nenhum espaço, é pq não precisa remover nada,
  // retorne a string sem mudar nada
  if (string::npos == first) {
    return str;
  }
  // Acha o último caractere que não é um espaço
  size_t last = str.find_last_not_of(' ');
  // Retorna a só os caracteres entre o first e o last
  return str.substr(first, (last - first + 1));
}

// Pega uma string e fatia ela nos pontos onde encontrarmos um caractere especificado
vector<string> split_string(const string& str, char split_point) {
  // Sei lá como isso funciona fui de stack overflow kkkk
  auto result = vector<string>{};
  auto ss = stringstream{str};

  for (string line; getline(ss, line, split_point);) {
    line = trim(line);
    result.push_back(line);
  }

  return result;
}

// Verifica se uma palavra é reservada para a nossa linguagem
bool eh_palavra_reservada(string palavra) {
  return palavra == "push" || palavra == "add" || palavra == "sub" ||
         palavra == "mul" || palavra == "div" || palavra == "store" ||
         palavra == "jmp" || palavra == "jeq" || palavra == "jgt" ||
         palavra == "jlt" || palavra == "in" || palavra == "out" ||
         palavra == "stp" || palavra == "pop";
}

// Garante que um nome de uma label é válido
void valida_nome_simbolo(string nome) {
  if (eh_palavra_reservada(nome)) {
    cout << "nome de label \"" << nome
         << "\" invalido, pois eh palavra reservada" << endl;
    // Se for palavra reservada, só explode o programa e sai
    exit(1);
  }
}

// Move as declarações de variáveis para o final do arquivo
// Então coisas como:
//     add 1
//     bananinha space 0
//     sub 2
// viram:
//     add 1
//     sub 2
//     bananinha space 0
vector<string> ordena_declaracoes(vector<string> linhas) {
  vector<string> linhas_normais;
  vector<string> linhas_space;

  // Para cada linha do programa...
  for (auto linha : linhas) {
    // Se a instrução for do tipo 'space', põe ela no vetor linhas_space;
    // do contrário, põe ela no vetor linhas_normais
    auto partes = split_string(linha, ' ');
    if (partes.size() == 3) {
      auto instrucao = partes[1];
      if (instrucao == "space") {
        linhas_space.push_back(linha);
      } else {
        linhas_normais.push_back(linha);
      }

    } else if (partes.size() == 2 && eh_palavra_reservada(partes[1])) {
      auto instrucao = partes[0];
      if (instrucao == "space") {
        linhas_space.push_back(linha);
      } else {
        linhas_normais.push_back(linha);
      }
    } else {
      linhas_normais.push_back(linha);
    }
  }

  // Concatena linhas normais com as linhas space
  linhas_normais.insert(
    linhas_normais.end(),
    linhas_space.begin(),
    linhas_space.end()
  );

  // Retorna resultado da concatenação
  return linhas_normais;
}

// Percorre o conteúdo procurando linhas do tipo "A B C", onde A são as labels
map<string, int> gera_tabela_de_simbolos(vector<string> conteudo) {
  auto result = map<string, int>{};
  int i = 0;

  // Para cada linha do programa...
  for (auto linha : conteudo) {
    // Separa a string por espaços
    auto partes = split_string(linha, ' ');
    if (partes.size() == 3 || partes.size() == 2 && eh_palavra_reservada(partes[1])) {
      // Se a label já foi declarada, explode e sai
      if (result.find(partes[0]) != result.end()) {
        cout << "erro: label \"" << partes[0] << "\" declarada mais de uma vez!" << endl;
        exit(1);
      }
      // Senão, registra a label
      result[partes[0]] = i;
      valida_nome_simbolo(partes[0]);
    }

    i++;
  }

  return result;
}

// Dada uma label, diz em qual linha ela foi declarada
int resolve_simbolo(string simbolo, map<string, int> simbolos) {
  if (simbolos.find(simbolo) != simbolos.end()) {
    return simbolos[simbolo];
  } else {
    try {
      return std::stoi(simbolo);
    } catch (const exception& e) {
      cout << "esperava um numero, encontrei \"" << simbolo
           << "\" (talvez voce tenha mencionado uma label que nao existe?)"
           << endl;
      exit(1);
    }
  }
}

// Traduz de assembly pra linguagem de máquina
string codificar_operacao_e_operando(string operacao, string operando,
                                     map<string, int> simbolos) {
  if (operacao == "push") {
    return "00 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "add") {
    return "01 00";
  } else if (operacao == "sub") {
    return "02 00";
  } else if (operacao == "mul") {
    return "03 00";
  } else if (operacao == "div") {
    return "04 00";
  } else if (operacao == "store" || operacao == "pop") {
    return "05 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "jmp") {
    return "06 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "jeq") {
    return "07 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "jgt") {
    return "08 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "jlt") {
    return "09 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "in") {
    return "10 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "out") {
    return "11 " + to_string(resolve_simbolo(operando, simbolos));
  } else if (operacao == "stp") {
    return "12 00";
  } else if (operacao == "space") {
    return to_string(resolve_simbolo(operando, simbolos)) + " 99";
  } else {
    cout << "erro de sintaxe, esperava instrucao, encontrei \"" << operacao
         << "\"" << endl;
    exit(1);
  }
}

// Dado um programa e sua tabela de símbolos, traduz cada instrução assembler
// em uma instrução de máquina
vector<string> monta_programa(vector<string> linhas,
                              map<string, int> simbolos) {
  auto result = vector<string>{};
  int i = 0;

  // Para cada linha do programa...
  for (auto linha : linhas) {
    auto partes = split_string(linha, ' ');
    string instrucao;

    // Vê quantas palavras tem na linha, e transforma essa linha em instrução de máquina
    switch (partes.size()) {
      case 1:
        instrucao = codificar_operacao_e_operando(partes[0], "", simbolos);
        break;
      case 2:
        if (eh_palavra_reservada(partes[1])) {
          instrucao = codificar_operacao_e_operando(partes[1], "", simbolos);
        } else {
          instrucao =
              codificar_operacao_e_operando(partes[0], partes[1], simbolos);
        }
        break;
      case 3:
        instrucao =
            codificar_operacao_e_operando(partes[1], partes[2], simbolos);
        break;
      default:
        cout << "erro de sintaxe na linha " << i + 1
             << ". Esperava uma instrução, encontrei \"" << linha << "\""
             << endl;
        exit(1);
        break;
    }
    // Concatena a instrução com as já geradas anteriormente e segue o loop
    result.push_back(instrucao);
    i++;
  }

  // Retorna as instruções de máquina concatenadas
  return result;
}

int main(int argc, char** argv) {
  if (argc < 2) {
    cout << "necessario o argumento de pelo menos um nome de arquivo!" << endl;
    return 1;
  }

  // Percorrer argumentos de linha de comandos
  // Abrir arquivos solicitados e concatenar todos juntos
  // Sei lá como, fui de stack overflow
  std::vector<string> linhas;
  for (int i = 1; i < argc; i++) {
    std::ifstream ifs(argv[i]);
    std::string conteudo((std::istreambuf_iterator<char>(ifs)),
                        (std::istreambuf_iterator<char>()));

    auto conteudo_splitado = split_string(conteudo, '\n');
    linhas.insert(linhas.end(), conteudo_splitado.begin(), conteudo_splitado.end());
  }

  // Joga as declarações de variável pro fim
  linhas = ordena_declaracoes(linhas);
  // Gera a tabela de símbolos
  auto simbolos = gera_tabela_de_simbolos(linhas);
  // Compila o programa
  auto programa = monta_programa(linhas, simbolos);

  // Escreve o programa compilado pra dentro de um arquivo
  std::ofstream outfile("out.mem");
  for (auto linha : programa) {
    outfile << linha << endl;
  }
  outfile.close();

  return 0;
}
