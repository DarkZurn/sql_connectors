#include <mysql.h>
#include <string>
#include <vector>

namespace sql_con {
  //Object work with Mariadb server
  class MariaDB_connector {
  public:
    MariaDB_connector(std::string host, std::string login, std::string pass) :	\
      host(host.c_str()), login(login.c_str()), pass(pass.c_str()){}

    MariaDB_connector(const char *host, const char *login, const char *pass) : \
      host(host), login(login), pass(pass){}

    ~MariaDB_connector() {}
    //Create new databese
    int create(const char *base) {
      if (con == NULL) {
	con = mysql_init(NULL);
      }
      if (mysql_real_connect(con, host, login, pass, \
			     NULL, 0, NULL, 0) == NULL) 
	{
	  finish_with_error(con);
	}
      char SQL[512];
      snprintf(SQL, sizeof SQL, "create database %s;", base);
      //Вставляем в базу
      insert_into_base(SQL);
      mysql_close(con);
    }
    
    //open existing database
    int open(std::string base) {
      //const char *base = base2.c_str();
      con = mysql_init(NULL);
  
      if (con == NULL) 
	{
	  fprintf(stderr, "%s\n", mysql_error(con));
	  exit(1);
	}  

      if (mysql_real_connect(con, host, login, pass, 
			     base.c_str(), 0, NULL, 0) == NULL) 
	{
	  finish_with_error(con);
	}    
      return 1;
    }

    //Закрыть соединение с базой
    int close() {
      mysql_close(con);
      return 0;
    }

    //Вставить в базу строку
    int insert(std::string SQL) {
      //Вставляем в базу
      insert_into_base(SQL.c_str());
    }

    //Получить одно целочисленное значение
    int get_one_int(std::string SQL) {
      //Отправляем запрос в базу
      if (mysql_query(con, SQL.c_str())) 
	{
	  finish_with_error(con);
	}   
      //Результат запроса
      MYSQL_RES *result = mysql_store_result(con);
    
      if (result == NULL) 
	{
	  finish_with_error(con);
	}
      //количество полученных колонок
      int num_fields = mysql_num_fields(result);
      //Строка запроса
      MYSQL_ROW row;
      int res = -1; //Возвращаемый результат
      //Пока не будут извлечены все строки
      while ((row = mysql_fetch_row(result))) 
	{
	  //Извлекаем колонки
	  for(int i = 0; i < num_fields; i++) 
	    {	    
	      res = atoi(row[i]);	    
	    } 	
	}  
      mysql_free_result(result);
      return res;
    }
    //Получитьс данные из базы в виде списка
    std::vector <std::string> get_one_column(std::string SQL) {
      //Отправляем запрос в базу
      if (mysql_query(con, SQL.c_str())) 
	{
	  finish_with_error(con);
	}   
      //Результат запроса
      MYSQL_RES *result = mysql_store_result(con);
      
      if (result == NULL) 
	{
	  finish_with_error(con);
	}
      //количество полученных колонок
      int num_fields = mysql_num_fields(result);      
      //Строка запроса
      MYSQL_ROW row;
      std::vector <std::string> list; //Возвращаемый результат
      //Пока не будут извлечены все строки
      while ((row = mysql_fetch_row(result))) 
	{
	  //Извлекаем колонки
	  for(int i = 0; i < num_fields; i++) 
	    {	    
	      list.push_back(row[i]);	    
	    } 	
	}  
      mysql_free_result(result);      
      return list;
    }
    //Получить таблицу из базы
    std::vector <std::vector<std::string>> select(std::string SQL, int head=0) {
      std::vector <std::vector<std::string>> table; //Возвращаемая таблица
      size_t j = 0;
      if (mysql_query(con, SQL.c_str()))
	{  
	  finish_with_error(con);
	}  
      MYSQL_RES *result = mysql_store_result(con);

      if (result == NULL) 
	{
	  finish_with_error(con);
	}
      int num_fields = mysql_num_fields(result);
      MYSQL_ROW row;
      MYSQL_FIELD *field;
  
      while ((row = mysql_fetch_row(result))) 
	{
	  table.push_back(std::vector<std::string>());
	  for(int i = 0; i < num_fields; i++) 
	    {
	      //Смотрим, нужно ли писать вместе с заголовками таблиц
	      if (i == 0 && head == 1) 
		{              
		  while(field = mysql_fetch_field(result)) 
		    {
		      //printf("%s ", field->name);
		      table[j].push_back(field->name);		      
		    }
		  table.push_back(std::vector<std::string>());
		  j++;
		}    
	      
	      table[j].push_back(row[i] ? row[i] : "NULL");
	    }
	  j++;
	}  
      mysql_free_result(result);
      return table;
    }

    //Получить список колонок таблицы
    std::vector <std::string> get_table_fields(std::string table) {
      std::string SQL = "select * from " + table + " limit 3;";
      //std::cout << SQL << std::endl;
      //Отправляем запрос в базу
      if (mysql_query(con, SQL.c_str())) 
	{
	  finish_with_error(con);
	}   
      //Результат запроса
      MYSQL_RES *result = mysql_store_result(con);
      MYSQL_FIELD *field;
      
      if (result == NULL) 
	{
	  finish_with_error(con);
	}
      //количество полученных колонок
      int num_fields = mysql_num_fields(result);      
      //Строка запроса
      MYSQL_ROW row;
      std::vector <std::string> list; //Возвращаемый результат
      while(field = mysql_fetch_field(result)) 
	{
	  //std::cout << field->name << std::endl;
	  list.push_back(field->name);
	}        
      mysql_free_result(result);      
      return list;
    }
    
    //-------------------
    MYSQL *con = NULL;
    const char *host; 
    const char *login; 
    const char *pass;
  
  private:
    void finish_with_error(MYSQL *con)
    {
      fprintf(stderr, "%s\n", mysql_error(con));
      mysql_close(con);
      exit(1);        
    }
    void insert_into_base(const char *SQL) {
      if (mysql_query(con, SQL)) {      
	finish_with_error(con);
      }
    }    
  };
}
