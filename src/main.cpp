#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <json.hpp>
#include <fstream>

#include "mylib.hpp"

using json = nlohmann::json;
using namespace std::chrono;

std::string getCurrentTime()
{
  auto now = system_clock::now();
  time_t now_c = system_clock::to_time_t(now);
  tm timeStruct;
  localtime_r(&now_c, &timeStruct); // Používá se na Linuxu (na Windows: localtime_s)

  std::stringstream ss;
  ss << std::put_time(&timeStruct, "%Y-%m-%dT%H:%M:%S");
  return ss.str();
}

typedef struct
{
  int id;
  std::string title;
  std::string content;
  std::string createdAt;
  std::string completedAt;
  bool completed;
} Task;

class TodoList
{
private:
  int idCounter = getLastId();
  std::string filename;

  int newId()
  {
    return ++idCounter;
  }

  int getLastId()
  {
    json tasksJson;
    std::ifstream fileIn(filename);
    if (fileIn.is_open())
    {
      fileIn >> tasksJson;
      fileIn.close();

      if (tasksJson.is_array() && !tasksJson.empty())
      {
        return tasksJson.back()["id"].get<int>(); // Vrátí poslední ID
      }
    }
    return 0; // Pokud není žádný úkol, ID začíná od 0
  }

public:
  TodoList(std::string _filename)
  {
    filename = _filename;
  }

  static void welcome()
  {
    std::cout << R"(
🔥🔥🔥 Aplikace ToDo List 🔥🔥🔥

Příkazy:
  A pro výpis všech úkolů
  C pro vytvoření úkolu
  Q pro ukončení aplikace
      
)";
  }

  char enterCommand()
  {
    std::cout << "Zadejte příkaz: ";
    char command;
    std::cin >> command;

    return command;
  }

  json getTasks()
  {
    json tasksJson;

    // Načti existující soubor, pokud existuje
    std::ifstream fileIn(filename);
    if (fileIn.is_open())
    {
      fileIn >> tasksJson;
      return tasksJson;
    }
    else
    {
      std::cerr << "Chyba při otevírání souboru!\n";
    }
  }

  json getTask(int taskId)
  {
    json tasksJson;

    // Načti existující soubor, pokud existuje
    std::ifstream fileIn(filename);
    if (fileIn.is_open())
    {
      fileIn >> tasksJson;
      for (const json &task : tasksJson)
      {
        task["id"] == taskId;
        return task;
      }
      std::cout << "Úkol s tímto ID nebyl nalezen.";
    }
    else
    {
      std::cerr << "Chyba při otevírání souboru!\n";
    }
  }

  void completeTask(int taskId)
  {
    json tasksJson;
    std::ifstream fileIn(filename);

    if (fileIn.is_open())
    {
      fileIn >> tasksJson;
      fileIn.close();

      bool found = false;

      if (tasksJson.is_array())
      {
        for (auto &task : tasksJson)
        {
          if (task["id"] == taskId)
          {
            if (task["completed"].get<bool>())
            {
              std::cout << "⚠️ Úkol s ID " << taskId << " už byl dokončen.\n";
              return;
            }

            task["completed"] = true;
            task["completedAt"] = getCurrentTime();
            found = true;
            break;
          }
        }
      }

      if (!found)
      {
        std::cout << "❌ Úkol s ID " << taskId << " nebyl nalezen.\n";
        return;
      }

      // Uložení aktualizovaných úkolů zpět do souboru
      std::ofstream fileOut(filename);
      if (fileOut.is_open())
      {
        fileOut << tasksJson.dump(4);
        fileOut.close();
        std::cout << "✅ Úkol s ID " << taskId << " byl označen jako hotový.\n";
      }
      else
      {
        std::cerr << "❌ Chyba při ukládání souboru!\n";
      }
    }
    else
    {
      std::cerr << "❌ Chyba při otevírání souboru!\n";
    }
  }

  void
  printTasks(json *tasksJson)
  {
    if (tasksJson->is_array() && !tasksJson->empty())
    {
      std::cout << "\n📋 Seznam úkolů:\n";
      for (const auto &task : *tasksJson)
      {
        printTask(task);
      }
      std::cout << "------------------------------\n";
    }
    else
    {
      std::cout << "📭 Žádné úkoly k zobrazení.\n";
    }
  }

  void printTask(json task)
  {
    std::cout << "------------------------------\n";
    std::cout << "🆔 ID: " << task["id"] << "\n";
    std::cout << "📌 Název: " << task["title"] << "\n";
    std::cout << "📝 Obsah: " << task["content"] << "\n";
    std::cout << "📅 Vytvořeno: " << task["createdAt"] << "\n";
    std::cout << "✅ Dokončeno: " << (task["completed"].get<bool>() ? "Ano" : "Ne") << "\n";
    if (task["completed"].get<bool>())
    {
      std::cout << "🏁 Dokončeno kdy: " << task["completedAt"] << "\n";
    }
  }

  void createTask()
  {
    Task newTask;

    std::cout << "Zadejte název úkolu: ";
    std::cin >> newTask.title;

    std::cout << "\nZadejte obsah úkolu: ";
    std::cin >> newTask.content;

    newTask.completed = false;
    newTask.createdAt = getCurrentTime();
    newTask.id = newId();

    saveTask(&newTask);
  };

  void saveTask(Task *task)
  {
    json tasksJson;

    // Načti existující soubor, pokud existuje
    std::ifstream fileIn(filename);
    if (fileIn.is_open())
    {
      fileIn >> tasksJson;
      fileIn.close();
    }

    // Pokud neexistuje "tasks", vytvoříme ho jako pole
    if (!tasksJson.is_array())
    {
      tasksJson = json::array();
    }

    // Přidání nového úkolu do JSON pole
    json taskJson = {
        {"id", task->id},
        {"title", task->title},
        {"content", task->content},
        {"completed", task->completed},
        {"completedAt", task->completedAt},
        {"createdAt", task->createdAt}};

    tasksJson.push_back(taskJson);

    // Zápis do souboru
    std::ofstream fileOut(filename);
    if (fileOut.is_open())
    {
      fileOut << tasksJson.dump(4); // Formátovaný zápis
      fileOut.close();
      std::cout << "Úkol byl uložen do souboru " << filename << std::endl;
    }
    else
    {
      std::cerr << "Chyba při otevírání souboru!" << std::endl;
    }
  }
};

int main()
{
  TodoList todoList("db.json");

  todoList.welcome();

  while (1)
  {
    Task newTask;
    json tasksJson;
    int taskId = 0;

    char command = todoList.enterCommand();

    switch (tolower(command))
    {
    case 'a':
      tasksJson = todoList.getTasks();
      todoList.printTasks(&tasksJson);
      break;
    case 'n':
      todoList.createTask();
      break;
    case 't':
      std::cout << "Zadejte ID úkolu: ";
      std::cin >> taskId;
      tasksJson = todoList.getTask(taskId);
      todoList.printTask(tasksJson);
      std::cout << "------------------------------\n";
      break;
    case 'c':
      std::cout << "Zadejte ID úkolu: ";
      std::cin >> taskId;
      todoList.completeTask(taskId);
      tasksJson = todoList.getTask(taskId);
      todoList.printTask(tasksJson);
      std::cout << "------------------------------\n";
      break;
    case 'q':
      exit(0);
    default:
      std::cout << "\n Neznámý příkaz";
      todoList.welcome();
      break;
    }
  }
  return 0;
}
