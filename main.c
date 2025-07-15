#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <curl/curl.h>
#include "textColors.h"

#define RETURN_FAILURE 100 // this is stupid


typedef struct {
	size_t size;
	char* response;
} Memory; 

char* get_api_key(){

	FILE* file = fopen("API_KEY", "r");
	if(!file){
		fprintf(stderr, "Failed to open API_KEY\n");
		return NULL;
	}

	char* key = malloc(128);
	if(!key){
		fprintf(stderr, "api key malloc\n");
		fclose(file);
		return NULL;
	}

	if(fgets(key, 128, file) == NULL){
		fprintf(stderr, "Couldn't read api key\n");
		free(key);
		fclose(file);
		return NULL;
	}

	fclose(file);

	// Remove trailing newline
	const size_t len = strlen(key);
	if(len > 0 && key[len-1] == '\n'){
		key[len-1] = '\0';
	}
	return key;
}

// Parse json response from API
double parse_response(Memory* chunk){
	
	cJSON* data = cJSON_Parse(chunk->response);
	if(!data){
		fprintf(stderr, "Failed to parse json\n");
		return RETURN_FAILURE;
	}

	// Get temperature info
	cJSON* current = cJSON_GetObjectItem(data, "current");
	if(!current){
		fprintf(stderr, "'Current' not found in json\n"
				"Did you make a typo?\n");
		cJSON_Delete(data);
		return RETURN_FAILURE;
	}

	cJSON* temp = cJSON_GetObjectItem(current, "heatindex_c");
	if(!temp || !cJSON_IsNumber(temp)){
		fprintf(stderr, "heatindex_c not found or NaN\n");
		cJSON_Delete(data);
		return RETURN_FAILURE;
	} 
	cJSON_Delete(data);
	return temp->valuedouble;
}

char* get_condition(Memory* chunk){
	
	cJSON* data = cJSON_Parse(chunk->response);
	if(!data){
		fprintf(stderr, "Failed to parse json\n");
		return RETURN_FAILURE;
	}

	// Get temperature info
	cJSON* current = cJSON_GetObjectItem(data, "current");
	if(!current){
		fprintf(stderr, "'Current' not found in json\n"
				"Did you make a typo?\n");
		cJSON_Delete(data);
		return RETURN_FAILURE;
	}
	cJSON* condition = cJSON_GetObjectItem(current, "condition");
	if(!condition){
		fprintf(stderr, "Field 'condition' not found in json\n");
		cJSON_Delete(data);
		return RETURN_FAILURE;
	}

	cJSON* condition_desc = cJSON_GetObjectItem(condition, "text");
	if(!condition_desc){
		fprintf(stderr, "Condition text not found in json\n");
		cJSON_Delete(data);
		return RETURN_FAILURE;
	}

	char* res = strdup(condition_desc->valuestring);
	cJSON_Delete(data);
	return res;
}

// Handle api data
size_t response_callback(
		void* content, size_t size,
		size_t nmemb, void* userp){
	
	size_t total_size = size*nmemb;
	Memory* mem = (Memory *)userp;

	char* ptr = realloc(mem->response, mem->size + total_size+1);
	if(ptr == NULL){
		printf("Not enough memory\n");
		return -1;
	}
	mem->response = ptr;
	memcpy(&(mem->response[mem->size]), content, total_size);
	mem->size += total_size;
	mem->response[mem->size] = '\0';
	return total_size;
}


int main(int argc, char** argv){

	char* city = malloc(32);
	strcpy(city, argv[1]);

	CURL* curl;
	CURLcode res;
	Memory chunk = {
		.response = malloc(1),
		.size = 0
	};

	// initialize curl
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	
	if(curl){
		char* api_key = get_api_key();
		char url[128];
		snprintf(url, sizeof(url),
				"http://api.weatherapi.com/v1/current.json?key=%s&q=%s&aqi=no", api_key, city);
		if(api_key == NULL){
			fprintf(stderr, "api key is null\n");
			exit(EXIT_FAILURE);
		}

		curl_easy_setopt(curl, CURLOPT_URL, url);

		// Follow redirect
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		
		// Callback setup shid
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
				response_callback);
		
		// Pass memory to callback 
		curl_easy_setopt(curl, CURLOPT_WRITEDATA,
				(void* )&chunk);

		// HTTP GET GOT
		res = curl_easy_perform(curl);

		if(res != CURLE_OK){
			fprintf(stderr, "Curl failed: %s\n",
					curl_easy_strerror(res));
		}/* else {
			printf("Response size: %zu\n", chunk.size);
		}*/
	
		free(api_key);
		curl_easy_cleanup(curl);
	}

	const double temp = parse_response(&chunk);
	const char* condition = get_condition(&chunk); // Caller must free
	int text_color;

	if(temp >= 25){
		text_color = 31; // Red
	} else if(temp >= 17){ 
		text_color = 33; // Yellow 
	} else {
		text_color = 32; // Green
	}
	change_color(text_color);
	printf(temp == RETURN_FAILURE ?
			"Something went wrong\n" :
			"Current temperature in %s: %.2f°C\n", city, temp);

	change_color(0);
	printf("Weather condition is %s\n", condition);

	free(chunk.response);
	free(city);
	free(condition);
        return 0;
}
