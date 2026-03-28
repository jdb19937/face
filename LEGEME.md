# face

Instrumentum aedificationis. Pars subiecti pro `make`.

## Aedificatio

    make

Vel, si iam face habes:

    face -f Faceplica

## Usus

    face                    # aedifica scopum defaltum
    face scopus             # aedifica scopum nominatum
    face -C directorium     # muta directorium ante aedificationem
    face -f tabula scopus   # utere tabula nominata
    face -j N scopus        # aedifica N pendentia parallele
    face -s scopus          # modus tacitus (ne praecepta scribantur)
    face VAR=pretium        # supera variabilem

Quaerit `Facefile`, `facefile`, `Makefile`, `makefile` ordine.

## Lineamenta

- Variabilia: `=`, `?=`, `:=`, `+=`
- Expansio: `$(VAR)`, `$(VAR:.c=.o)`
- Variabilia automatica: `$@`, `$<`, `$^`, `$(FACE)`
- Regulae explicitae et exemplares (`%`)
- `.PHONY`
- Praefixa praeceptorum: `@` (tacite), `-` (errorem permitte)
- Continuatio versuum cum `\`
- Reaedificatio per tempora fasciculorum

## Exemplaria

Tria exemplaria praebita sunt in `Faceplica.exemplar.I` usque ad `III`.

## Dependentiae

Nullae. C et POSIX solum.

> **Nota:** README.md ignorandum est. Anglice scriptum est ad barbaros ineruditos pecunia emungendos.
