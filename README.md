# Лабораторная работа 3: Обратный вызов. События и точки подключения.

## 1. Задание
Необходимо в ранее созданный компонент добавить точки подключения с интерфейсом событий демонстрирующих пошаговую работу алгоритма выполненного задания. Для выполнения задания используется предоставленный шаблон с точками подключения.

## 2. Реализация

### События сортировки CombSort

Компонент `CEcoLab1` генерирует четыре типа событий, позволяющих отслеживать процесс выполнения алгоритма сортировки расческой:

1. **OnCombSortStart** — событие начала сортировки. Передаёт количество элементов в массиве `n`. Срабатывает перед началом сортировки в функции `CombSort`.

2. **OnCombSortIterationStart** — событие начала итерации сортировки. Срабатывает при начале каждой новой итерации внешнего цикла. Передаёт общее количество элементов `n`, номер текущей итерации `iteration` и текущий размер зазора `gap`.

3. **OnCombSortIterationComplete** — событие завершения итерации. Срабатывает после завершения прохода по массиву с текущим зазором. Передаёт общее количество элементов `n`, номер итерации `iteration` и количество обменов в текущей итерации `swaps_in_iteration`.

4. **OnCombSortComplete** — событие завершения сортировки. Содержит общее количество элементов `n` и общее количество обменов `swap_count`. Срабатывает после полного завершения алгоритма.

### Интерфейс событий

```c
typedef struct IEcoLab1VTblEvents {
    /* IEcoUnknown */
    int16_t (ECOCALLMETHOD *QueryInterface )(/* in */ struct IEcoLab1Events* me, /* in */ const UGUID* riid, /* out */ void **ppv);
    uint32_t (ECOCALLMETHOD *AddRef )(/* in */ struct IEcoLab1Events* me);
    uint32_t (ECOCALLMETHOD *Release )(/* in */ struct IEcoLab1Events* me);

    /* IEcoLab1Events */
    int16_t (ECOCALLMETHOD *OnCombSortStart)(/* in */ struct IEcoLab1Events* me, /* in */ size_t n);
    int16_t (ECOCALLMETHOD *OnCombSortIterationStart)(/* in */ struct IEcoLab1Events* me, /* in */ size_t n, /* in */ int32_t iteration, /* in */ size_t gap);
    int16_t (ECOCALLMETHOD *OnCombSortIterationComplete)(/* in */ struct IEcoLab1Events* me, /* in */ size_t n, /* in */ int32_t iteration, /* in */ int32_t swaps_in_iteration);
    int16_t (ECOCALLMETHOD *OnCombSortComplete)(/* in */ struct IEcoLab1Events* me, /* in */ size_t n, /* in */ int32_t swap_count);
}
```

### Пример реализации обработчика событий

```c
int16_t ECOCALLMETHOD CEcoLab1Sink_OnCombSortStart(/* in */ struct IEcoLab1Events* me, /* in */ size_t n) {
    CEcoLab1Sink* pCMe = (CEcoLab1Sink*)me;

    if (me == 0 ) {
        return -1;
    }

    pCMe->m_iStep++;
    printf("Print from Sink. Step %d: CombSort started for array with size %d\n", pCMe->m_iStep, n);

    return 0;
}
```

## 3. Интеграция через точку подключения

### Настройка точки подключения

Компонент `CEcoLab1` реализует интерфейс `IEcoConnectionPointContainer`, что позволяет внешним объектам подписываться на события. Инициализация точки подключения происходит в функции `initCEcoLab1`:

```c
/* Создание точки подключения */
result = createCEcoLab1ConnectionPoint((IEcoUnknown*)pCMe->m_pISys, (IEcoConnectionPointContainer*)pCMe, &IID_IEcoLab1Events, (IEcoConnectionPoint**)&((pCMe)->m_pISinkCP));
```

Это обеспечивает возможность подключения нескольких подписчиков одновременно, что расширяет возможности мониторинга процесса сортировки.

### Генерация событий в алгоритме

Алгоритм сортировки CombSort реализован с генерацией событий на ключевых этапах выполнения:

1. **Начало сортировки**: генерируется событие `OnCombSortStart` перед началом основного цикла.
2. **Начало итерации**: перед каждым проходом по массиву генерируется `OnCombSortIterationStart`.
3. **Завершение итерации**: после каждого прохода генерируется `OnCombSortIterationComplete` с количеством обменов.
4. **Завершение сортировки**: после полного завершения алгоритма генерируется `OnCombSortComplete`.

**Фрагмент реализации:**

```c
static int16_t ECOCALLMETHOD CEcoLab1_CombSort(/* in */ IEcoLab1Ptr_t me, 
                                               /* in */ void* base, 
                                               /* in */ size_t num, 
                                               /* in */ size_t size, 
                                               /* in */ int (ECOCDECLMETHOD *compare)(const void*, const void*)) {
    CEcoLab1* pCMe = (CEcoLab1*)me;
    size_t gap = num;
    int swapped = 1;
    int32_t swap_count = 0;
    int32_t swaps_in_iteration = 0;
    int32_t iteration = 0;

    // Проверка на корректность входных данных
    if (me == 0 || base == 0 || num == 0 || size == 0 || compare == 0) {
        return ERR_ECO_POINTER;
    }

    // Событие начала сортировки
    CEcoLab1_Fire_OnCombSortStart(me, num);

    // Основной цикл сортировки расческой
    while (gap > 1 || swapped) {
        iteration++;
        
        // Вычисление следующего зазора
        gap = (size_t)((double)gap / 1.3);
        if (gap < 1) {
            gap = 1;
        }

        // Событие начала итерации
        CEcoLab1_Fire_OnCombSortIterationStart(me, num, iteration, gap);

        swaps_in_iteration = 0;
        swapped = 0;

        // Проход по массиву с текущим зазором
        for (size_t i = 0; i + gap < num; i++) {
            // Сравнение и обмен элементов
            if (compare((char*)base + i * size, (char*)base + (i + gap) * size) > 0) {
                // Обмен элементов
                swapped = 1;
                swap_count++;
                swaps_in_iteration++;
            }
        }

        // Событие завершения итерации
        CEcoLab1_Fire_OnCombSortIterationComplete(me, num, iteration, swaps_in_iteration);
    }

    // Событие завершения сортировки
    CEcoLab1_Fire_OnCombSortComplete(me, num, swap_count);
    return ERR_ECO_SUCCESES;
}
```

### Функции генерации событий

Функции генерации событий (`CEcoLab1_Fire_OnCombSortStart`, `CEcoLab1_Fire_OnCombSortIterationStart`, `CEcoLab1_Fire_OnCombSortIterationComplete`, `CEcoLab1_Fire_OnCombSortComplete`) используют механизм перечисления подключений для доставки событий всем зарегистрированным подписчикам:

```c
static int16_t ECOCALLMETHOD CEcoLab1_Fire_OnCombSortStart(/* in */ struct IEcoLab1* me, /* in */ size_t n) {
    CEcoLab1* pCMe = (CEcoLab1*)me;
    int16_t result = 0;
    IEcoEnumConnections* pEnum = 0;
    IEcoLab1Events* pIEvents = 0;
    EcoConnectionData cd;

    if (me == 0 ) {
        return -1;
    }

    if (pCMe->m_pISinkCP != 0) {
        result = ((IEcoConnectionPoint*)pCMe->m_pISinkCP)->pVTbl->EnumConnections((IEcoConnectionPoint*)pCMe->m_pISinkCP, &pEnum);
        if ( (result == 0) && (pEnum != 0) ) {
            while (pEnum->pVTbl->Next(pEnum, 1, &cd, 0) == 0) {
                result = cd.pUnk->pVTbl->QueryInterface(cd.pUnk, &IID_IEcoLab1Events, (void**)&pIEvents);
                if ( (result == 0) && (pIEvents != 0) ) {
                    result = pIEvents->pVTbl->OnCombSortStart(pIEvents, n);
                    pIEvents->pVTbl->Release(pIEvents);
                }
                cd.pUnk->pVTbl->Release(cd.pUnk);
            }
            pEnum->pVTbl->Release(pEnum);
        }
    }
    return result;
}
```

## 4. Использование

### Подключение к событиям

Для получения событий необходимо создать объект, реализующий интерфейс `IEcoLab1Events`, и подключить его к компоненту через точку подключения:

```c
/* Создание sink для обработки событий */
CEcoLab1Sink* pSink = 0;
IEcoLab1Events* pIEcoLab1Events = 0;
result = createCEcoLab1Sink(pIMem, &pIEcoLab1Events);

/* Подключение к событиям */
pSink = (CEcoLab1Sink*)pIEcoLab1Events;
result = pSink->Advise(pSink, pIEcoLab1);

/* Выполнение сортировки - события будут генерироваться автоматически */
int32_t array[] = {5, 2, 8, 1, 9};
pIEcoLab1->pVTbl->CombSort(pIEcoLab1, array, 5, sizeof(int32_t), comparator);

/* Отключение от событий */
pSink->Unadvise(pSink, pIEcoLab1);
```

### Пример вывода событий
<img width="581" height="709" alt="image" src="https://github.com/user-attachments/assets/130b1aee-b4dc-4405-b808-6bfde70cf7a8" />


## 5. Вывод

Реализованная система позволяет отслеживать пошаговое выполнение алгоритма сортировки CombSort, 
предоставляя детальную информацию о каждой итерации, изменении зазора и количестве обменов элементов.
Это обеспечивает прозрачность работы алгоритма и упрощает отладку и анализ производительности.
Механизм событий формирует прозрачную модель взаимодействия, где компонент не требует прямого опроса от клиента.
Благодаря такой архитектуре внешний клиент получает возможность реагировать на происходящее в реальном времени.
