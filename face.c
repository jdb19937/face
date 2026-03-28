/*
 * face — instrumentum aedificationis
 *
 * Pars subiecti pro 'make'. POSIX, nullae dependentiae externae.
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

/* ============================================================
 * I. LIMITES ET TYPI
 * ============================================================ */

#define LIM_FILUM       131072
#define LIM_VERSUS      8192
#define LIM_DICTA       1024
#define LIM_REGULAE     4096
#define LIM_FORMAE      512
#define LIM_PRAECEPTA   256
#define LIM_FICTA       512
#define LIM_VERBA       4096
#define LIM_PROFUNDITAS 64
#define LIM_AEDIFICATA  4096

/* contextus variabilium automaticarum ($@, $<, $^) */
typedef struct {
	const char *scopus;       /* $@ */
	const char *primum;       /* $< */
	const char *omnia;        /* $^ */
} contextus_t;

/* dictum — coniugatio variabilis (nomen = pretium) */
typedef struct {
	char *nomen;
	char *pretium;
	int immutabilis;          /* positum per lineam imperatam */
} dictum_t;

/* regula — regula aedificationis explicita */
typedef struct {
	char *scopus;             /* nomen scopi (expansum) */
	char *pendentia_cruda;    /* pendentia cruda (non expansa) */
	char *praecepta[LIM_PRAECEPTA];
	int num_praecepta;
} regula_t;

/* forma — regula exemplaris (cum %) */
typedef struct {
	char *exemplar;           /* e.g. "%.o" vel "arcana/%.o" */
	char *pendentia_cruda;    /* pendentia cruda cum % */
	char *praecepta[LIM_PRAECEPTA];
	int num_praecepta;
} forma_t;

/* ============================================================
 * II. STATUS GLOBALIS
 * ============================================================ */

static dictum_t dicta[LIM_DICTA];
static int      num_dicta;

static regula_t regulae[LIM_REGULAE];
static int      num_regulae;

static forma_t  formae[LIM_FORMAE];
static int      num_formae;

static char    *ficta[LIM_FICTA];
static int      num_ficta;

static char    *scopus_primus;

static int      modus_tacitus;

/* semita aedificationis currens (ad circulos detegendos) */
static char    *semita[LIM_AEDIFICATA];
static int      num_semitae;

/* scopi iam aedificati (ne bis aedificentur) */
static char    *iam[LIM_AEDIFICATA];
static int      num_iam;

/* ============================================================
 * III. UTILITATES
 * ============================================================ */

static void mori(const char *nuntius)
{
	fprintf(stderr, "face: %s\n", nuntius);
	exit(2);
}

static char *duplica(const char *plicam)
{
	if (!plicam) return NULL;
	char *novum = strdup(plicam);
	if (!novum) mori("memoria exhausta");
	return novum;
}

/* tonde spatia ab utroque fine — mutat plicam in loco */
static char *tonde(char *plicam)
{
	while (*plicam && isspace((unsigned char)*plicam)) plicam++;
	char *finis = plicam + strlen(plicam);
	while (finis > plicam && isspace((unsigned char)finis[-1])) finis--;
	*finis = '\0';
	return plicam;
}

/* scinde plicam per spatia in verba; reddit numerum verborum */
static int scinde(const char *plicam, char **verba, int lim)
{
	int n = 0;
	const char *p = plicam;
	while (*p && n < lim) {
		while (*p && isspace((unsigned char)*p)) p++;
		if (!*p) break;
		const char *ab = p;
		while (*p && !isspace((unsigned char)*p)) p++;
		size_t lon = (size_t)(p - ab);
		verba[n] = malloc(lon + 1);
		if (!verba[n]) mori("memoria exhausta");
		memcpy(verba[n], ab, lon);
		verba[n][lon] = '\0';
		n++;
	}
	return n;
}

/* ============================================================
 * IV. DICTA (VARIABILIA)
 * ============================================================ */

static dictum_t *quaere_dictum(const char *nomen)
{
	for (int i = 0; i < num_dicta; i++)
		if (strcmp(dicta[i].nomen, nomen) == 0)
			return &dicta[i];
	return NULL;
}

static void pone_dictum(const char *nomen, const char *pretium, int immutabilis)
{
	dictum_t *d = quaere_dictum(nomen);
	if (d) {
		if (d->immutabilis) return;
		free(d->pretium);
		d->pretium = duplica(pretium);
		d->immutabilis = immutabilis;
		return;
	}
	if (num_dicta >= LIM_DICTA) mori("nimis multa dicta");
	dicta[num_dicta].nomen = duplica(nomen);
	dicta[num_dicta].pretium = duplica(pretium);
	dicta[num_dicta].immutabilis = immutabilis;
	num_dicta++;
}

/* ?= — pone solum si nondum definitum */
static void pone_si_vacuum(const char *nomen, const char *pretium)
{
	if (!quaere_dictum(nomen))
		pone_dictum(nomen, pretium, 0);
}

/* ============================================================
 * V. EXPANSIO VARIABILIUM
 * ============================================================ */

static char *expande(const char *plicam, const contextus_t *ctx, int prof);

/* substitue suffixum: pro quoque verbo, si finitur cum 'vetus',
 * muta finem in 'novum' */
static char *substitue_suffixum(const char *plicam,
                                const char *vetus, const char *novum)
{
	size_t lon_vet = strlen(vetus);
	size_t lon_nov = strlen(novum);
	char res[LIM_FILUM];
	size_t pos = 0;
	const char *p = plicam;
	int primum = 1;

	while (*p) {
		while (*p && isspace((unsigned char)*p)) p++;
		if (!*p) break;
		const char *ab = p;
		while (*p && !isspace((unsigned char)*p)) p++;
		size_t lon_v = (size_t)(p - ab);

		if (!primum && pos < LIM_FILUM - 1) res[pos++] = ' ';
		primum = 0;

		if (lon_v >= lon_vet &&
		    memcmp(ab + lon_v - lon_vet, vetus, lon_vet) == 0) {
			size_t lon_prae = lon_v - lon_vet;
			if (pos + lon_prae + lon_nov >= LIM_FILUM - 1)
				mori("substitutio nimis longa");
			memcpy(res + pos, ab, lon_prae);
			pos += lon_prae;
			memcpy(res + pos, novum, lon_nov);
			pos += lon_nov;
		} else {
			if (pos + lon_v >= LIM_FILUM - 1)
				mori("substitutio nimis longa");
			memcpy(res + pos, ab, lon_v);
			pos += lon_v;
		}
	}
	res[pos] = '\0';
	return duplica(res);
}

static void scribe_in(char *res, size_t *pos, const char *add, size_t lon)
{
	if (*pos + lon >= LIM_FILUM - 1) mori("expansio nimis longa");
	memcpy(res + *pos, add, lon);
	*pos += lon;
}

static void scribe_plicam(char *res, size_t *pos, const char *add)
{
	scribe_in(res, pos, add, strlen(add));
}

static char *expande(const char *plicam, const contextus_t *ctx, int prof)
{
	if (prof > LIM_PROFUNDITAS) mori("expansio nimis profunda (recursio?)");
	if (!plicam) return duplica("");

	char res[LIM_FILUM];
	size_t pos = 0;
	const char *p = plicam;

	while (*p) {
		if (*p != '$') {
			if (pos >= LIM_FILUM - 2) mori("expansio nimis longa");
			res[pos++] = *p++;
			continue;
		}
		p++; /* transi '$' */
		if (!*p) { res[pos++] = '$'; break; }

		if (*p == '$') {
			/* $$ -> $ */
			res[pos++] = '$';
			p++;
		} else if (*p == '@') {
			if (ctx && ctx->scopus)
				scribe_plicam(res, &pos, ctx->scopus);
			p++;
		} else if (*p == '<') {
			if (ctx && ctx->primum)
				scribe_plicam(res, &pos, ctx->primum);
			p++;
		} else if (*p == '^') {
			if (ctx && ctx->omnia)
				scribe_plicam(res, &pos, ctx->omnia);
			p++;
		} else if (*p == '(' || *p == '{') {
			/* $(VAR) vel $(VAR:vet=nov) */
			p++;
			const char *ab = p;
			int alt = 1;
			while (*p && alt > 0) {
				if (*p == '(' || *p == '{') alt++;
				else if (*p == ')' || *p == '}') alt--;
				if (alt > 0) p++;
			}
			if (alt != 0) mori("parenthesis non clausa in expansione");

			size_t lon_c = (size_t)(p - ab);
			char cont[LIM_FILUM];
			if (lon_c >= LIM_FILUM) mori("nomen variabilis nimis longum");
			memcpy(cont, ab, lon_c);
			cont[lon_c] = '\0';
			p++; /* transi ')' vel '}' */

			/* quaere ':' pro substitutione suffixorum
			 * (extra $() nidificatas) */
			char *colon = NULL;
			{
				int d = 0;
				for (char *q = cont; *q; q++) {
					if (*q == '$' && (q[1] == '(' || q[1] == '{'))
						{ d++; q++; continue; }
					if (d > 0 && (*q == ')' || *q == '}'))
						{ d--; continue; }
					if (d == 0 && *q == ':')
						{ colon = q; break; }
				}
			}

			if (colon) {
				/* $(VAR:vetus=novum) */
				*colon = '\0';
				char *nom = cont;
				char *spec = colon + 1;
				char *aeq = strchr(spec, '=');
				if (!aeq) mori("'=' deest in substitutione");
				*aeq = '\0';
				char *vetus = spec;
				char *novum = aeq + 1;

				dictum_t *d = quaere_dictum(nom);
				if (d) {
					char *val = expande(d->pretium, ctx, prof + 1);
					char *sub = substitue_suffixum(val, vetus, novum);
					scribe_plicam(res, &pos, sub);
					free(val);
					free(sub);
				}
			} else {
				/* $(VAR) simplex */
				dictum_t *d = quaere_dictum(cont);
				if (d) {
					char *val = expande(d->pretium, ctx, prof + 1);
					scribe_plicam(res, &pos, val);
					free(val);
				}
				/* si non inventum, expanditur in vacuum */
			}
		} else {
			/* $X — variabilis unius litterae */
			char nom[2] = { *p, '\0' };
			dictum_t *d = quaere_dictum(nom);
			if (d) {
				char *val = expande(d->pretium, ctx, prof + 1);
				scribe_plicam(res, &pos, val);
				free(val);
			}
			p++;
		}
	}
	res[pos] = '\0';
	return duplica(res);
}

/* ============================================================
 * VI. FORMAE (EXEMPLARIA CUM %)
 * ============================================================ */

/* congrue exemplar cum scopo; reddit stirpem si congruit, aliter NULL.
 * vocans debet liberare plicam reditum. */
static char *congrue(const char *scopus, const char *exemplar)
{
	const char *pct = strchr(exemplar, '%');
	if (!pct) {
		if (strcmp(scopus, exemplar) == 0) return duplica("");
		return NULL;
	}
	size_t lon_prae = (size_t)(pct - exemplar);
	size_t lon_suf  = strlen(pct + 1);
	size_t lon_scop = strlen(scopus);

	if (lon_scop < lon_prae + lon_suf) return NULL;
	if (lon_prae > 0 && memcmp(scopus, exemplar, lon_prae) != 0) return NULL;
	if (lon_suf  > 0 &&
	    memcmp(scopus + lon_scop - lon_suf, pct + 1, lon_suf) != 0) return NULL;

	size_t lon_st = lon_scop - lon_prae - lon_suf;
	char *stirps = malloc(lon_st + 1);
	if (!stirps) mori("memoria exhausta");
	memcpy(stirps, scopus + lon_prae, lon_st);
	stirps[lon_st] = '\0';
	return stirps;
}

/* substitue '%' cum stirpe in filo */
static char *applica_stirpem(const char *plicam, const char *stirps)
{
	char res[LIM_FILUM];
	size_t pos = 0;
	size_t lon_st = strlen(stirps);
	const char *p = plicam;

	while (*p) {
		if (*p == '%') {
			if (pos + lon_st >= LIM_FILUM - 1) mori("stirps nimis longa");
			memcpy(res + pos, stirps, lon_st);
			pos += lon_st;
			p++;
		} else {
			if (pos >= LIM_FILUM - 2) mori("applicatio nimis longa");
			res[pos++] = *p++;
		}
	}
	res[pos] = '\0';
	return duplica(res);
}

/* ============================================================
 * VII. TEMPUS FASCICULI
 * ============================================================ */

static time_t tempus_fasciculi(const char *via)
{
	struct stat st;
	if (stat(via, &st) != 0) return 0;
	return st.st_mtime;
}

/* ============================================================
 * VIII. FICTA (.PHONY)
 * ============================================================ */

static int est_fictum(const char *scopus)
{
	for (int i = 0; i < num_ficta; i++)
		if (strcmp(ficta[i], scopus) == 0) return 1;
	return 0;
}

static void adde_fictum(const char *scopus)
{
	if (est_fictum(scopus)) return;
	if (num_ficta >= LIM_FICTA) mori("nimis multa ficta");
	ficta[num_ficta++] = duplica(scopus);
}

/* ============================================================
 * IX. RESOLUTIO TABULAE (PARSING)
 * ============================================================ */

/* tolle commentum: '#' extra $() truncat lineam */
static void tolle_commentum(char *linea)
{
	int alt = 0;
	for (char *p = linea; *p; p++) {
		if (*p == '$' && (p[1] == '(' || p[1] == '{'))
			{ alt++; p++; continue; }
		if (alt > 0 && (*p == ')' || *p == '}'))
			{ alt--; continue; }
		if (alt == 0 && *p == '#')
			{ *p = '\0'; return; }
	}
}

static void lege_tabulam(const char *via)
{
	FILE *f = fopen(via, "r");
	if (!f) {
		fprintf(stderr, "face: '%s' aperire non possum: %s\n",
		        via, strerror(errno));
		exit(2);
	}

	/* lege versus physicos, coniunge continuationes (\) */
	char *versus[LIM_VERSUS];
	int num_versus = 0;

	char acervus[LIM_FILUM];
	size_t lon_ac = 0;
	acervus[0] = '\0';

	char linea[LIM_FILUM];
	while (fgets(linea, sizeof(linea), f)) {
		size_t lon = strlen(linea);
		if (lon > 0 && linea[lon - 1] == '\n') linea[--lon] = '\0';
		if (lon > 0 && linea[lon - 1] == '\r') linea[--lon] = '\0';

		if (lon > 0 && linea[lon - 1] == '\\') {
			/* continuatio */
			linea[--lon] = '\0';
			if (lon_ac + lon + 1 >= LIM_FILUM)
				mori("versus nimis longus");
			memcpy(acervus + lon_ac, linea, lon);
			lon_ac += lon;
			acervus[lon_ac] = '\0';
			continue;
		}

		if (lon_ac + lon >= LIM_FILUM) mori("versus nimis longus");
		memcpy(acervus + lon_ac, linea, lon + 1);

		if (num_versus >= LIM_VERSUS) mori("nimis multi versus");
		versus[num_versus++] = duplica(acervus);
		acervus[0] = '\0';
		lon_ac = 0;
	}
	if (lon_ac > 0) {
		if (num_versus >= LIM_VERSUS) mori("nimis multi versus");
		versus[num_versus++] = duplica(acervus);
	}
	fclose(f);

	/* tracta versus logicos */
	regula_t *reg_currens = NULL;
	forma_t  *for_currens = NULL;

	for (int i = 0; i < num_versus; i++) {
		char *v = versus[i];

		/* praeceptum (incipit cum tabula)? */
		if (v[0] == '\t') {
			char *prc = v + 1;
			if (reg_currens) {
				if (reg_currens->num_praecepta >= LIM_PRAECEPTA)
					mori("nimis multa praecepta");
				reg_currens->praecepta[reg_currens->num_praecepta++] =
					duplica(prc);
			} else if (for_currens) {
				if (for_currens->num_praecepta >= LIM_PRAECEPTA)
					mori("nimis multa praecepta");
				for_currens->praecepta[for_currens->num_praecepta++] =
					duplica(prc);
			}
			/* si nullum contextum habemus, linea ignoratur */
			continue;
		}

		/* non praeceptum — frange contextum */
		reg_currens = NULL;
		for_currens = NULL;

		/* tolle commentum et tonde */
		tolle_commentum(v);
		char *t = tonde(v);
		if (*t == '\0') continue;

		/* determina typum: assignatio (=) vel regula (:) */
		const char *p = t;
		int alt = 0;
		int typus = 0;       /* 0=ignotum, 1= '=', 2=regula, 3='?=', 4='+=' */
		const char *sep = NULL;
		int lon_op = 0;

		while (*p) {
			if (*p == '$' && (p[1] == '(' || p[1] == '{'))
				{ alt++; p += 2; continue; }
			if (alt > 0 && (*p == ')' || *p == '}'))
				{ alt--; p++; continue; }
			if (alt == 0) {
				if (*p == '?' && p[1] == '=')
					{ typus = 3; sep = p; lon_op = 2; break; }
				if (*p == '+' && p[1] == '=')
					{ typus = 4; sep = p; lon_op = 2; break; }
				if (*p == ':' && p[1] == '=')
					{ typus = 1; sep = p; lon_op = 2; break; }
				if (*p == '=')
					{ typus = 1; sep = p; lon_op = 1; break; }
				if (*p == ':')
					{ typus = 2; sep = p; lon_op = 1; break; }
			}
			p++;
		}

		if (typus == 1 || typus == 3 || typus == 4) {
			/* assignatio variabilis */
			size_t lon_nom = (size_t)(sep - t);
			char nomen[LIM_FILUM];
			if (lon_nom >= LIM_FILUM) mori("nomen nimis longum");
			memcpy(nomen, t, lon_nom);
			nomen[lon_nom] = '\0';
			char *nom = tonde(nomen);

			const char *pret = sep + lon_op;
			while (*pret && isspace((unsigned char)*pret)) pret++;
			char pretium[LIM_FILUM];
			strncpy(pretium, pret, LIM_FILUM - 1);
			pretium[LIM_FILUM - 1] = '\0';
			tonde(pretium);

			if (typus == 3) {
				pone_si_vacuum(nom, pretium);
			} else if (typus == 4) {
				dictum_t *d = quaere_dictum(nom);
				if (d) {
					char coniunctum[LIM_FILUM];
					snprintf(coniunctum, sizeof(coniunctum),
					         "%s %s", d->pretium, pretium);
					pone_dictum(nom, coniunctum, 0);
				} else {
					pone_dictum(nom, pretium, 0);
				}
			} else {
				pone_dictum(nom, pretium, 0);
			}

		} else if (typus == 2) {
			/* regula aedificationis */
			size_t lon_sc = (size_t)(sep - t);
			char scopus_crudus[LIM_FILUM];
			if (lon_sc >= LIM_FILUM) mori("scopus nimis longus");
			memcpy(scopus_crudus, t, lon_sc);
			scopus_crudus[lon_sc] = '\0';
			char *sc = tonde(scopus_crudus);

			const char *pend = sep + 1;
			while (*pend && isspace((unsigned char)*pend)) pend++;
			char pendentia[LIM_FILUM];
			strncpy(pendentia, pend, LIM_FILUM - 1);
			pendentia[LIM_FILUM - 1] = '\0';
			tonde(pendentia);

			/* expande scopum (pro $(NOMEN) etc.) */
			char *scopus_exp = expande(sc, NULL, 0);
			char *scop = tonde(scopus_exp);

			/* .PHONY */
			if (strcmp(scop, ".PHONY") == 0) {
				char *ficta_exp = expande(pendentia, NULL, 0);
				char *verba[LIM_VERBA];
				int n = scinde(ficta_exp, verba, LIM_VERBA);
				for (int j = 0; j < n; j++) {
					adde_fictum(verba[j]);
					free(verba[j]);
				}
				free(ficta_exp);
				free(scopus_exp);
				continue;
			}

			/* forma exemplaris (%) vel regula explicita? */
			if (strchr(scop, '%')) {
				if (num_formae >= LIM_FORMAE)
					mori("nimis multae formae");
				forma_t *fo = &formae[num_formae++];
				fo->exemplar = duplica(scop);
				fo->pendentia_cruda = duplica(pendentia);
				fo->num_praecepta = 0;
				for_currens = fo;
			} else {
				/* quaere regulam iam existentem pro hoc scopo */
				regula_t *existens = NULL;
				for (int j = 0; j < num_regulae; j++) {
					if (strcmp(regulae[j].scopus, scop) == 0) {
						existens = &regulae[j];
						break;
					}
				}
				if (existens) {
					/* coniunge pendentia */
					if (*pendentia) {
						char con[LIM_FILUM];
						snprintf(con, sizeof(con), "%s %s",
						         existens->pendentia_cruda,
						         pendentia);
						free(existens->pendentia_cruda);
						existens->pendentia_cruda = duplica(con);
					}
					reg_currens = existens;
				} else {
					if (num_regulae >= LIM_REGULAE)
						mori("nimis multae regulae");
					regula_t *r = &regulae[num_regulae++];
					r->scopus = duplica(scop);
					r->pendentia_cruda = duplica(pendentia);
					r->num_praecepta = 0;
					reg_currens = r;

					/* primum scopum explicitum serva */
					if (!scopus_primus)
						scopus_primus = duplica(scop);
				}
			}
			free(scopus_exp);
		}
		/* linea ignota ignoratur */
	}

	for (int i = 0; i < num_versus; i++) free(versus[i]);
}

/* ============================================================
 * X. AEDIFICATIO
 * ============================================================ */

static int aedifica(const char *scopus);

static regula_t *quaere_regulam(const char *scopus)
{
	for (int i = 0; i < num_regulae; i++)
		if (strcmp(regulae[i].scopus, scopus) == 0)
			return &regulae[i];
	return NULL;
}

/* quaere formam optimam (stirpem brevissimam praeferens) */
static forma_t *quaere_formam(const char *scopus, char **stirps_p)
{
	forma_t *optima = NULL;
	char    *stirps_opt = NULL;
	size_t   lon_min = (size_t)-1;

	for (int i = 0; i < num_formae; i++) {
		char *s = congrue(scopus, formae[i].exemplar);
		if (s) {
			size_t lon = strlen(s);
			if (lon < lon_min) {
				free(stirps_opt);
				stirps_opt = s;
				lon_min = lon;
				optima = &formae[i];
			} else {
				free(s);
			}
		}
	}
	if (optima) {
		*stirps_p = stirps_opt;
		return optima;
	}
	return NULL;
}

/* exsequere praeceptum unum per /bin/sh */
static void exsequere(const char *praeceptum)
{
	const char *p = praeceptum;
	int tacite = 0;
	int permitte = 0;

	while (*p == '@' || *p == '-') {
		if (*p == '@') tacite = 1;
		if (*p == '-') permitte = 1;
		p++;
	}

	if (!tacite && !modus_tacitus) {
		printf("%s\n", p);
		fflush(stdout);
	}

	int status = system(p);
	if (status == -1) {
		fprintf(stderr, "face: exsequi non possum: %s\n", p);
		if (!permitte) exit(2);
		return;
	}
	if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
		if (!permitte) {
			fprintf(stderr, "face: *** Error %d\n",
			        WEXITSTATUS(status));
			exit(2);
		}
	} else if (WIFSIGNALED(status)) {
		if (!permitte) {
			fprintf(stderr, "face: *** Signum %d\n",
			        WTERMSIG(status));
			exit(2);
		}
	}
}

static int iam_aedificatum(const char *scopus)
{
	for (int i = 0; i < num_iam; i++)
		if (strcmp(iam[i], scopus) == 0) return 1;
	return 0;
}

static void nota_aedificatum(const char *scopus)
{
	if (num_iam >= LIM_AEDIFICATA) mori("nimis multa aedificata");
	iam[num_iam++] = duplica(scopus);
}

static int aedifica(const char *scopus)
{
	/* si iam aedificatum, nihil agendum */
	if (iam_aedificatum(scopus)) return 0;

	/* detege circulos */
	for (int i = 0; i < num_semitae; i++) {
		if (strcmp(semita[i], scopus) == 0) {
			fprintf(stderr, "face: circulus detectus: '%s'\n",
			        scopus);
			exit(2);
		}
	}
	if (num_semitae >= LIM_AEDIFICATA)
		mori("semita nimis profunda");
	int idx = num_semitae;
	semita[num_semitae++] = duplica(scopus);

	/* quaere regulam explicitam */
	regula_t *regula = quaere_regulam(scopus);
	char *stirps = NULL;
	forma_t *forma = NULL;

	char *pend_cruda = NULL;
	char **praecepta = NULL;
	int    num_praecepta = 0;
	int    pend_allocata = 0;   /* an pend_cruda liberanda sit */

	if (regula) {
		pend_cruda = regula->pendentia_cruda;
		praecepta = regula->praecepta;
		num_praecepta = regula->num_praecepta;
	}

	/* si nulla praecepta, quaere formam */
	if (!regula || num_praecepta == 0) {
		forma = quaere_formam(scopus, &stirps);
		if (forma) {
			char *pend_st = applica_stirpem(
				forma->pendentia_cruda, stirps);
			if (!regula) {
				pend_cruda = pend_st;
				pend_allocata = 1;
			} else {
				/* coniunge pendentia regulae et formae */
				char con[LIM_FILUM];
				snprintf(con, sizeof(con), "%s %s",
				         regula->pendentia_cruda, pend_st);
				free(pend_st);
				pend_cruda = duplica(con);
				pend_allocata = 1;
			}
			praecepta = forma->praecepta;
			num_praecepta = forma->num_praecepta;
		}
	}

	/* si nulla regula et fasciculus non existit */
	if (!regula && !forma) {
		if (tempus_fasciculi(scopus) == 0) {
			fprintf(stderr,
			        "face: regulam pro '%s' invenire non possum\n",
			        scopus);
			exit(2);
		}
		/* fasciculus fontis — nihil aedificandum */
		free(semita[idx]);
		num_semitae = idx;
		nota_aedificatum(scopus);
		return 0;
	}

	/* expande pendentia */
	char *pend_exp = pend_cruda ?
		expande(pend_cruda, NULL, 0) : duplica("");

	/* scinde in verba singula */
	char *verba[LIM_VERBA];
	int num_pend = scinde(pend_exp, verba, LIM_VERBA);

	/* aedifica omnia pendentia */
	time_t tempus_max = 0;
	for (int i = 0; i < num_pend; i++) {
		aedifica(verba[i]);
		time_t tp = tempus_fasciculi(verba[i]);
		if (tp > tempus_max) tempus_max = tp;
	}

	/* an debeamus praecepta exsequi? */
	int fictum = est_fictum(scopus);
	time_t tempus_scopi = tempus_fasciculi(scopus);
	int debet = 0;

	if (fictum)
		debet = 1;
	else if (tempus_scopi == 0)
		debet = 1;
	else if (tempus_max > tempus_scopi)
		debet = 1;

	if (debet && num_praecepta > 0) {
		/* para variabilia automatica */
		char prim[LIM_FILUM] = "";
		if (num_pend > 0)
			strncpy(prim, verba[0], LIM_FILUM - 1);

		char omn[LIM_FILUM];
		size_t opos = 0;
		omn[0] = '\0';
		for (int i = 0; i < num_pend; i++) {
			if (i > 0 && opos < LIM_FILUM - 2)
				omn[opos++] = ' ';
			size_t lon = strlen(verba[i]);
			if (opos + lon >= LIM_FILUM - 1) break;
			memcpy(omn + opos, verba[i], lon);
			opos += lon;
		}
		omn[opos] = '\0';

		contextus_t ctx = { scopus, prim, omn };

		for (int i = 0; i < num_praecepta; i++) {
			char *exp = expande(praecepta[i], &ctx, 0);
			exsequere(exp);
			free(exp);
		}
	}

	/* purga */
	free(semita[idx]);
	num_semitae = idx;
	for (int i = 0; i < num_pend; i++) free(verba[i]);
	free(pend_exp);
	if (pend_allocata) free(pend_cruda);
	free(stirps);

	nota_aedificatum(scopus);
	return 0;
}

/* ============================================================
 * XI. PRINCEPS
 * ============================================================ */

int main(int argc, char **argv)
{
	const char *via = NULL;
	const char *directorium = NULL;
	char *scopi[256];
	int num_scopi = 0;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-C") == 0) {
			if (++i >= argc) mori("-C: nomen directorii deest");
			directorium = argv[i];
		} else if (strcmp(argv[i], "-f") == 0) {
			if (++i >= argc) mori("-f: nomen tabulae deest");
			via = argv[i];
		} else if (strcmp(argv[i], "-s") == 0 ||
		           strcmp(argv[i], "--silent") == 0) {
			modus_tacitus = 1;
		} else if (strchr(argv[i], '=')) {
			/* VAR=pretium in linea imperata */
			char *dup = duplica(argv[i]);
			char *aeq = strchr(dup, '=');
			*aeq = '\0';
			pone_dictum(dup, aeq + 1, 1);
			free(dup);
		} else {
			if (num_scopi >= 256) mori("nimis multi scopi");
			scopi[num_scopi++] = argv[i];
		}
	}

	/* muta directorium si -C datum est */
	if (directorium) {
		if (chdir(directorium) != 0) {
			fprintf(stderr, "face: in '%s' intrare non possum: %s\n",
			        directorium, strerror(errno));
			exit(2);
		}
	}

	/* quaere tabulam */
	if (!via) {
		if      (access("Faceplica", R_OK) == 0) via = "Faceplica";
		else if (access("faceplica", R_OK) == 0) via = "faceplica";
		// else if (access("Makefile", R_OK) == 0)  via = "Makefile";
		// else if (access("makefile", R_OK) == 0 ) via = "makefile";
		// else mori("nec Faceplica nec Makefile invenire possum");
		// — sub condicione approbationis 
		else mori("Faceplica invenire non possum");
	}

	lege_tabulam(via);

	if (num_scopi == 0) {
		if (!scopus_primus)
			mori("nullum scopum invenire possum");
		scopi[0] = scopus_primus;
		num_scopi = 1;
	}

	for (int i = 0; i < num_scopi; i++)
		aedifica(scopi[i]);

	return 0;
}
