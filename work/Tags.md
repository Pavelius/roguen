# Описание заголовков

### Признаки / Feats

Группа признаков, которые могут иметь любое существо или предмет. В файле они идут в разделе `feats`. Преемущество признаков их небольшой размер и наличие или остуствие признака не несет никаких накладных расходов по хранению. Каждый признак занимает 1 бит. То есть 32 признака будут занимать 4 байта - стандартный размер одного целого числа.

- `TwoHanded`: Предмет необходимо держать в двух руках. То есть во вторую руку взять ничего не получиться

- `CutWoods`: Оружие с этим признаком может рубить деревья. Шанс срубить дерево равен урону с модификатором от силы или удвоенному значению, если оружие двуручное.

- `Retaliate`: При удачном парироании этим оружием оно нносит урон в ответ.

- `Thrown`: Оружие ближего боя с этим признаком можно бросить во врага. Вы его теряете, но оно наносит урон на расстоянии.

- `Coins`: Предмет с єтим признаком не существует в инвентаре отдельнім предметом. В то время когда вы его берете он тут же добавляет свою стоимость к общему знчению денег персонажа.

- `Female`: Признак сущности женского пола. Может менять окончанию у строк, согласно правилам грамматике русского языка. На него можно ориентироваться в условиях.

- `Undead`: Существо оживший мертвец. Обычно на него не действуют заклинания лечения, а также некоторые материалы наносят двойной урон.

- `Summoned`: Существо вызвано на помощь. Когда происходит смена локации оно исчезает. Временное существо.

- `Ally`: Существо сражается **за** игрока и атакует его врагов если увидит.

- `Enemy`: Существо сражается **против** игрока и атакует его или его союзников если увидит.

- `Stun`: штраф -10% к навыкам ближнего боя. Каждый свой ход существо тестирует силу чтобы избавится от этого состояния.
