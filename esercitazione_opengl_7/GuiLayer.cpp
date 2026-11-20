#include "GuiLayer.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

GuiLayer::GuiLayer(GLFWwindow* window, Scene& scene,
                   const std::array<std::size_t, 5>& geometryIds)
    : scene_(scene), geometryIds_(geometryIds) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}
GuiLayer::~GuiLayer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
void GuiLayer::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
void GuiLayer::draw() {
    ImGui::Begin("Scena 3D - controlli");
    drawSettings(); ImGui::Separator();
    drawAddObject(); ImGui::Separator();
    drawObjectList(); ImGui::Separator();
    drawSelectedObject(); ImGui::Separator();
    ImGui::Text("Operazione: %s", operation_.c_str());
    ImGui::Text("Asse: %s", axis_.c_str());
    ImGui::TextWrapped("TAB: navigazione | Trascina: orbita camera");
    ImGui::TextWrapped("1/2/3: asse X/Y/Z | T/R/G: trasla/ruota/scala | Canc: elimina");
    ImGui::TextWrapped("WASD/QE: camera | Frecce: orbita | Spazio: wireframe");
    ImGui::End();
}
void GuiLayer::drawSettings() {
    // TODO 1 - Impostazioni generali della scena.
    // Inserire i widget ImGui per modificare i membri privati:
    //   background_      -> ImGui::ColorEdit4
    ImGui::ColorEdit4("Sfondo", &background_.x);
    //   wireframe_       -> ImGui::Checkbox
    ImGui::Checkbox("Wireframe", &wireframe_);
    //   showAnchor_      -> ImGui::Checkbox
    ImGui::Checkbox("Mostra Ancora", &showAnchor_);
    //   navigationMode_  -> ImGui::Checkbox
    ImGui::Checkbox("Navigazione", &navigationMode_);
    // Suggerimento: il primo float di un glm::vec4 e' &background_.x.
    ImGui::TextDisabled("TODO 1: impostazioni della scena");
}

void GuiLayer::drawAddObject() {
    // TODO 2 - Selezione della primitiva e pulsante di creazione.
    // Tipi disponibili: Cubo, Piano, Piramide, Sfera, Toro.
    const char* names[] = { "Cubo", "Piano", "Piramide", "Sfera", "Toro" };
    // 1. Definire i nomi delle cinque primitive.
    ImGui::Combo("Nuova primitiva", &type_, names, 5);
    // 2. Usare ImGui::Combo per modificare type_.
    // 3. Usare ImGui::Button e chiamare addObject(type_) al click.
    if (ImGui::Button("Aggiungi oggetto")) addObject(type_);
}

void GuiLayer::addObject(int type) {
    // TODO 3 - Creare una nuova ISTANZA senza duplicare la geometria.
    // 1. Controllare che type sia compreso nell'intervallo [0, 4].
    const char* names[] = { "Cubo", "Piano", "Piramide", "Sfera", "Toro" };
    if (type < 0 || type >= 5) return;
    // 2. Costruire un nome progressivo con counters_[type].
    std::string name = std::string(names[type]) + " " + std::to_string(++counters_[type]);
    // 3. Usare scene_.addInstance(geometryIds_[type], name).
    std::size_t id = scene_.addInstance(geometryIds_[type], name);
    // 4. Accedere alla nuova Instance, assegnare position=(0,1,0),
    //    aggiornare la Model Matrix e impostare selectable=true.
    Instance& object = scene_.instance(id);
    object.position = glm::vec3(0.0f, 1.0f, 0.0f);
    object.updateModelMatrix();
    // 5. Memorizzare l'indice della nuova istanza in selected_.
    selected_ = id;
    (void)type; // Eliminarlo quando si utilizza il parametro.
}

void GuiLayer::drawObjectList() {
    // TODO 4 - Elenco e selezione delle istanze della scena.
    // 1. Scorrere scene_.instances() utilizzando gli indici.
    // 2. Ignorare le istanze con selectable == false.
    // 3. Usare ImGui::PushID / ImGui::Selectable / ImGui::PopID.
    // 4. Quando si clicca un nome, aggiornare selected_.
    ImGui::Text("Elenco delle istanze selezionabili");
    for (int i = 0; i < scene_.instances().size(); i++) {
        const Instance& object = scene_.instance(i);

        if (!object.selectable) continue;

        ImGui::PushID(i);
        bool isSelected = selected_ == i;

        if (ImGui::Selectable(object.name.c_str(), isSelected)) {
            selected_ = i;
        }

        ImGui::PopID();
    }
}

void GuiLayer::drawSelectedObject() {
    // TODO 5 - Proprietà dell'istanza selezionata.
    // 1. Verificare che selected_ sia un indice valido; altrimenti
    //    mostrare "Nessun oggetto selezionato" e terminare il metodo.
    // 2. Ottenere Instance& mediante scene_.instance(selected_).
    // 3. Aggiungere controlli per visible, color, useUniformColor.
    // 4. Usare ImGui::DragFloat3 per position, rotation e scale.
    // 5. Se cambiano i parametri TRS, chiamare updateModelMatrix().
    // 6. Aggiungere un pulsante per eliminare l'istanza e riportare
    //    selected_ a -1 (nessuna selezione).
    // Suggerimento: mantenere scale >= 0.01 per ciascun asse.
    ImGui::Text("Proprieta' dell'oggetto selezionato");

    if (selected_ < 0 || selected_ >= scene_.instances().size()) {
        ImGui::Text("Nessun oggetto selezionato");
        return;
    }

    Instance& selected = scene_.instance(selected_);

    ImGui::Text("Oggetto: %s", selected.name.c_str());
    if (ImGui::ColorEdit3("Colore oggetto ", &selected.color.x)) {
        selected.useUniformColor = true;
    }
    ImGui::Checkbox("Usa colore uniforme", &selected.useUniformColor);

    bool changedPosition = ImGui::DragFloat3("Posizione", &selected.position.x, 0.1f);
    bool changedRotation = ImGui::DragFloat3("Rotazione", &selected.rotation.x, 0.1f);
    bool changedScale = ImGui::DragFloat3("Scala", &selected.scale.x, 0.1f);
    selected.updateModelMatrix();

    if (ImGui::Button("Elimina selezionato")) {
        scene_.removeInstance(selected_);
        selected_ = -1;

        return;
    }
}

void GuiLayer::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
