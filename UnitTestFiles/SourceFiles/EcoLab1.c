/*
 * <кодировка символов>
 *   Cyrillic (UTF-8 with signature) - Codepage 65001
 * </кодировка символов>
 *
 * <сводка>
 *   EcoLab1
 * </сводка>
 *
 * <описание>
 *   Данный исходный файл является точкой входа
 * </описание>
 *
 * <автор>
 *   Copyright (c) 2018 Vladimir Bashev. All rights reserved.
 * </автор>
 *
 */


/* Eco OS */
#include "IEcoSystem1.h"
#include "IdEcoMemoryManager1.h"
#include "IdEcoInterfaceBus1.h"
#include "IdEcoFileSystemManagement1.h"
#include "IdEcoLab1.h"
#include "IdEcoList1.h"
#include "CEcoLab1Sink.h"
#include <time.h>

int ECOCDECLMETHOD comparator(const void* a, const void* b) {
    int32_t int_a = *((const int32_t*)a);
    int32_t int_b = *((const int32_t*)b);
    return (int_a > int_b) - (int_a < int_b);
}

/*
 *
 * <сводка>
 *   Функция EcoMain
 * </сводка>
 *
 * <описание>
 *   Функция EcoMain - точка входа
 * </описание>
 *
 */
int16_t EcoMain(IEcoUnknown* pIUnk) {
    int16_t result = -1;
    /* Указатель на системный интерфейс */
    IEcoSystem1* pISys = 0;
    /* Указатель на интерфейс работы с системной интерфейсной шиной */
    IEcoInterfaceBus1* pIBus = 0;
    /* Указатель на интерфейс работы с памятью */
    IEcoMemoryAllocator1* pIMem = 0;

    int32_t* combArray = 0;
    int32_t* qsortArray = 0;
    uint32_t n = 0;
    uint32_t i = 0;
    uint32_t k = 0;
    clock_t combBegin, combEnd, qsortBegin, qsortEnd;

    /* Указатель на тестируемый интерфейс */
    IEcoLab1* pIEcoLab1 = 0;
    IEcoLab1Events* pIEcoLab1Events = 0;
    CEcoLab1Sink* pSink = 0;

    /* Проверка и создание системного интрефейса */
    if (pISys == 0) {
        result = pIUnk->pVTbl->QueryInterface(pIUnk, &GID_IEcoSystem, (void **)&pISys);
        if (result != 0 && pISys == 0) {
        /* Освобождение системного интерфейса в случае ошибки */
            goto Release;
        }
    }

    /* Получение интерфейса для работы с интерфейсной шиной */
    result = pISys->pVTbl->QueryInterface(pISys, &IID_IEcoInterfaceBus1, (void **)&pIBus);
    if (result != 0 || pIBus == 0) {
        /* Освобождение в случае ошибки */
        goto Release;
    }
#ifdef ECO_LIB
    /* Регистрация статического компонента для работы со списком */
    result = pIBus->pVTbl->RegisterComponent(pIBus, &CID_EcoLab1, (IEcoUnknown*)GetIEcoComponentFactoryPtr_1F5DF16EE1BF43B999A434ED38FE8F3A);
    if (result != 0 ) {
        /* Освобождение в случае ошибки */
        goto Release;
    }
#endif
    /* Получение интерфейса управления памятью */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoMemoryManager1, 0, &IID_IEcoMemoryAllocator1, (void**) &pIMem);

    /* Проверка */
    if (result != 0 || pIMem == 0) {
        /* Освобождение системного интерфейса в случае ошибки */
        goto Release;
    }

    /* Получение тестируемого интерфейса */
    result = pIBus->pVTbl->QueryComponent(pIBus, &CID_EcoLab1, 0, &IID_IEcoLab1, (void**) &pIEcoLab1);
    if (result != 0 || pIEcoLab1 == 0) {
        /* Освобождение интерфейсов в случае ошибки */
        goto Release;
    }

    /* Создание sink для обработки событий */
    result = createCEcoLab1Sink(pIMem, &pIEcoLab1Events);
    if (result != 0 || pIEcoLab1Events == 0) {
        goto Release;
    }

    pSink = (CEcoLab1Sink*)pIEcoLab1Events;

    /* Подключение к событиям */
    result = pSink->Advise(pSink, pIEcoLab1);
    if (result != 0) {
        goto Release;
    }

    k = 0;
    while (k <= 20) {
        n = (uint32_t)pow(2, k);

        combArray = (int32_t*)pIMem->pVTbl->Alloc(pIMem, n * sizeof(int32_t));
        qsortArray = (int32_t*)pIMem->pVTbl->Alloc(pIMem, n * sizeof(int32_t));
        for (i = 0; i < n; i++) {
            int32_t randomValue = rand();
            combArray[i] = randomValue;
            qsortArray[i] = randomValue;
        }

        combBegin = clock();
        pIEcoLab1->pVTbl->CombSort(pIEcoLab1, combArray, n, sizeof(int32_t), comparator);
        combEnd = clock();

        qsortBegin = clock();
        qsort(qsortArray, n, sizeof(int32_t), comparator);
        qsortEnd = clock();

        printf("Print from UnitTest. n=%u; comb_time=%u; qsort_time=%u\n", n, combEnd - combBegin, qsortEnd - qsortBegin);

        /* Освобождение памяти */
        pIMem->pVTbl->Free(pIMem, combArray);
        pIMem->pVTbl->Free(pIMem, qsortArray);
        combArray = 0;
        qsortArray = 0;

        k++;
    }
    system("Pause");

Release:
    /* Отключение от событий */
    if (pSink != 0 && pIEcoLab1 != 0) {
        pSink->Unadvise(pSink, pIEcoLab1);
    }
    if (pIEcoLab1Events != 0) pIEcoLab1Events->pVTbl->Release(pIEcoLab1Events);
    /* Освобождение интерфейса для работы с интерфейсной шиной */
    if (pIBus != 0) {
        pIBus->pVTbl->Release(pIBus);
    }

    /* Освобождение интерфейса работы с памятью */
    if (pIMem != 0) {
        pIMem->pVTbl->Release(pIMem);
    }

    /* Освобождение тестируемого интерфейса */
    if (pIEcoLab1 != 0) {
        pIEcoLab1->pVTbl->Release(pIEcoLab1);
    }


    /* Освобождение системного интерфейса */
    if (pISys != 0) {
        pISys->pVTbl->Release(pISys);
    }

    return result;
}

