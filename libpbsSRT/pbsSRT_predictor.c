#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pbsSRT_predictor.h"

typedef struct pbsSRT_predictor_template_s
{
    char                *name;
    const struct pbsSRT_predictor_template_s *next;
    const pbsSRT_predictor_t  predictor;
} pbsSRT_predictor_template_t;

const pbsSRT_predictor_template_t mavslmsbank_template
=
{
    .name = "mavslmsbank",
    .next = NULL,
    .predictor = 
    {
        .alloc  =   pbsSRT_alloc_MAVSLMSBank,
        .init   =   pbsSRT_init_MAVSLMSBank,
        .update =   pbsSRT_update_MAVSLMSBank,
        .predict=   pbsSRT_predict_MAVSLMSBank,
        .free   =   pbsSRT_free_MAVSLMSBank 
        }
};

const pbsSRT_predictor_template_t mabank_template
=
{
    .name = "mabank",
    .next = &mavslmsbank_template,
    .predictor = 
    {
        .alloc  =   pbsSRT_alloc_MABank,
        .init   =   pbsSRT_init_MABank,
        .update =   pbsSRT_update_MABank,
        .predict=   pbsSRT_predict_MABank,
        .free   =   pbsSRT_free_MABank 
        }
};

const pbsSRT_predictor_template_t template_template
=
{
    .name = "template",
    .next = &mabank_template,
    .predictor = 
    {
        .alloc  =   pbsSRT_alloc_template,
        .init   =   pbsSRT_init_template,
        .update =   pbsSRT_update_template,
        .predict=   pbsSRT_predict_template,
        .free   =   pbsSRT_free_template 
        }
};

const pbsSRT_predictor_template_t head
=
{
    .name = "Head",
    .next = &template_template,
    .predictor = {0}
};

int pbsSRT_getPredictor(pbsSRT_predictor_t *ppredictor, char *name)
{
    int ret;
    
    int found = 0;
    const pbsSRT_predictor_template_t *current = &head;

    *ppredictor = (pbsSRT_predictor_t){0};
    
    while(NULL != current->next)
    {
        current = current->next;
        
        ret = strcmp(current->name, name);
        if(0 == ret)
        {
            /*Copying the entire struct*/
            *ppredictor = current->predictor;
            found = 1;
            break;
        }
    }
    
    if(0 != found)
    {
        ppredictor->state = ppredictor->alloc();
        if(NULL == ppredictor->state)
        {
            fprintf(stderr, "pbsSRT_getPredictor: The \"alloc\" function failed for "
                            "\"%s\".\n", name);
            ret = 1;
            goto exit0;
        }
        
        ppredictor->init(ppredictor->state);
    }
    else
    {
        fprintf(stderr, "pbsSRT_getPredictor: No predictor named \"%s\" was found.\n",
                        name);
        ret = -1;
        goto exit0;
    }
    
exit0:
    return ret;
}

