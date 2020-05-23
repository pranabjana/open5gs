
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "link.h"

OpenAPI_link_t *OpenAPI_link_create(
    char *href
    )
{
    OpenAPI_link_t *link_local_var = OpenAPI_malloc(sizeof(OpenAPI_link_t));
    if (!link_local_var) {
        return NULL;
    }
    link_local_var->href = href;

    return link_local_var;
}

void OpenAPI_link_free(OpenAPI_link_t *link)
{
    if (NULL == link) {
        return;
    }
    OpenAPI_lnode_t *node;
    ogs_free(link->href);
    ogs_free(link);
}

cJSON *OpenAPI_link_convertToJSON(OpenAPI_link_t *link)
{
    cJSON *item = NULL;

    if (link == NULL) {
        ogs_error("OpenAPI_link_convertToJSON() failed [Link]");
        return NULL;
    }

    item = cJSON_CreateObject();
    if (link->href) {
        if (cJSON_AddStringToObject(item, "href", link->href) == NULL) {
            ogs_error("OpenAPI_link_convertToJSON() failed [href]");
            goto end;
        }
    }

end:
    return item;
}

OpenAPI_link_t *OpenAPI_link_parseFromJSON(cJSON *linkJSON)
{
    OpenAPI_link_t *link_local_var = NULL;
    cJSON *href = cJSON_GetObjectItemCaseSensitive(linkJSON, "href");

    if (href) {
        if (!cJSON_IsString(href)) {
            ogs_error("OpenAPI_link_parseFromJSON() failed [href]");
            goto end;
        }
    }

    link_local_var = OpenAPI_link_create (
        href ? ogs_strdup(href->valuestring) : NULL
        );

    return link_local_var;
end:
    return NULL;
}
