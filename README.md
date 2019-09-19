# ec020-exercicios-cap4-parte2

A IDE utilizada nesse projeto foi o [MCUXpresso](https://www.nxp.com/design/software/development-software/mcuxpresso-software-and-tools/mcuxpresso-integrated-development-environment-ide:MCUXpresso-IDE) (que pode ser baixado por [aqui](https://www.nxp.com/webapp/swlicensing/sso/downloadSoftware.sp?catid=MCUXPRESSO)).

## 1. Abrir o workspace
Isso pode ser feito de duas maneiras:
- Com o MCUXpresso já aberto, abrir a pasta `workspace` como workspace (`File` -> `Switch Workspace` -> `Other`).
- Na inicialização do MCUXpresso, abre uma janela de seleção de workspace. Selecione a pasta `workspace` desse repositório e clique em `Launch`.

## 2. Importar os projetos
1. Clique em `File` -> `Open Projects from File System...`.
2. Clique em `Directory`, navegue para pasta `workspace`.
3. Desmarque o item `workspace` da lista de projetos a serem importados e clique em `Finish`.
  ![Passos para importar os projetos](importar_projetos.png "Passos para importar os projetos")

## 3. Buildar as dependências
Antes de rodar o `Projeto`, certifique-se de selecionar o projeto `demo` e clicar em `Build` (no painel `Quickstart Panel` que fica no lado inferior esquerdo da tela) para gerar os arquivos necessários para o `Projeto`.