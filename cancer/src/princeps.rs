/*
 * face — instrumentum aedificationis
 *
 * Translatio Rustica fidelis ex C originali.
 * Nullae dependentiae externae. Ut Bernoulli: ex primis principiis.
 *
 * CANCER = CRUSTACEUS = RUST 🦀
 */

use std::env;
use std::fs;
use std::os::unix::fs::MetadataExt;
use std::process::{self, Command};

/* ============================================================
 * I. LIMITES ET TYPI
 * ============================================================ */

const LIM_FILUM: usize = 131072;
const LIM_PRAECEPTA: usize = 256;
const LIM_PROFUNDITAS: usize = 64;

/* contextus variabilium automaticarum ($@, $<, $^) */
struct Contextus {
    scopus: String, /* $@ */
    primum: String, /* $< */
    omnia: String,  /* $^ */
}

/* dictum — coniugatio variabilis (nomen = pretium) */
struct Dictum {
    nomen: String,
    pretium: String,
    immutabilis: bool, /* positum per lineam imperatam */
}

/* regula — regula aedificationis explicita */
struct Regula {
    scopus: String,          /* nomen scopi (expansum) */
    pendentia_cruda: String, /* pendentia cruda (non expansa) */
    praecepta: Vec<String>,
}

/* forma — regula exemplaris (cum %) */
struct Forma {
    exemplar: String,        /* e.g. "%.o" vel "arcana/%.o" */
    pendentia_cruda: String, /* pendentia cruda cum % */
    praecepta: Vec<String>,
}

/* ============================================================
 * II. STATUS GLOBALIS
 * ============================================================ */

struct Status {
    dicta: Vec<Dictum>,
    regulae: Vec<Regula>,
    formae: Vec<Forma>,
    ficta: Vec<String>,
    scopus_primus: Option<String>,
    modus_tacitus: bool,
    numerus_operariorum: usize, /* -j: maximum processuum parallelorum */
    semita: Vec<String>,        /* ad circulos detegendos */
    iam: Vec<String>,           /* scopi iam aedificati */
}

impl Status {
    fn novus() -> Self {
        Status {
            dicta: Vec::new(),
            regulae: Vec::new(),
            formae: Vec::new(),
            ficta: Vec::new(),
            scopus_primus: None,
            modus_tacitus: false,
            numerus_operariorum: 1,
            semita: Vec::new(),
            iam: Vec::new(),
        }
    }
}

/* ============================================================
 * III. UTILITATES
 * ============================================================ */

fn mori(nuntius: &str) -> ! {
    eprintln!("face: {}", nuntius);
    process::exit(2);
}

/* tonde spatia ab utroque fine */
fn tonde(plicam: &str) -> &str {
    plicam.trim()
}

/* scinde plicam per spatia in verba */
fn scinde(plicam: &str) -> Vec<String> {
    plicam.split_whitespace().map(|v| v.to_string()).collect()
}

/* ============================================================
 * IV. DICTA (VARIABILIA)
 * ============================================================ */

impl Status {
    fn quaere_dictum(&self, nomen: &str) -> Option<&Dictum> {
        self.dicta.iter().find(|d| d.nomen == nomen)
    }

    fn quaere_dictum_mut(&mut self, nomen: &str) -> Option<&mut Dictum> {
        self.dicta.iter_mut().find(|d| d.nomen == nomen)
    }

    fn pone_dictum(&mut self, nomen: &str, pretium: &str, immutabilis: bool) {
        if let Some(d) = self.quaere_dictum_mut(nomen) {
            if d.immutabilis {
                return;
            }
            d.pretium = pretium.to_string();
            d.immutabilis = immutabilis;
            return;
        }
        self.dicta.push(Dictum {
            nomen: nomen.to_string(),
            pretium: pretium.to_string(),
            immutabilis,
        });
    }

    /* ?= — pone solum si nondum definitum */
    fn pone_si_vacuum(&mut self, nomen: &str, pretium: &str) {
        if self.quaere_dictum(nomen).is_none() {
            self.pone_dictum(nomen, pretium, false);
        }
    }
}

/* ============================================================
 * V. EXPANSIO VARIABILIUM
 * ============================================================ */

/* substitue suffixum: pro quoque verbo, si finitur cum 'vetus',
 * muta finem in 'novum' */
fn substitue_suffixum(plicam: &str, vetus: &str, novum: &str) -> String {
    let mut res = String::new();
    let mut primum = true;
    for verbum in plicam.split_whitespace() {
        if !primum {
            res.push(' ');
        }
        primum = false;
        if verbum.ends_with(vetus) {
            let praefixum = &verbum[..verbum.len() - vetus.len()];
            res.push_str(praefixum);
            res.push_str(novum);
        } else {
            res.push_str(verbum);
        }
    }
    res
}

impl Status {
    fn expande(&self, plicam: &str, ctx: Option<&Contextus>, prof: usize) -> String {
        if prof > LIM_PROFUNDITAS {
            mori("expansio nimis profunda (recursio?)");
        }

        let bytes = plicam.as_bytes();
        let mut res = String::with_capacity(plicam.len());
        let mut i = 0;
        let lon = bytes.len();

        while i < lon {
            if bytes[i] != b'$' {
                res.push(bytes[i] as char);
                i += 1;
                continue;
            }
            i += 1; /* transi '$' */
            if i >= lon {
                res.push('$');
                break;
            }

            match bytes[i] {
                b'$' => {
                    /* $$ -> $ */
                    res.push('$');
                    i += 1;
                }
                b'@' => {
                    if let Some(c) = ctx {
                        res.push_str(&c.scopus);
                    }
                    i += 1;
                }
                b'<' => {
                    if let Some(c) = ctx {
                        res.push_str(&c.primum);
                    }
                    i += 1;
                }
                b'^' => {
                    if let Some(c) = ctx {
                        res.push_str(&c.omnia);
                    }
                    i += 1;
                }
                b'(' | b'{' => {
                    /* $(VAR) vel $(VAR:vet=nov) */
                    i += 1;
                    let ab = i;
                    let mut alt: i32 = 1;
                    while i < lon && alt > 0 {
                        match bytes[i] {
                            b'(' | b'{' => alt += 1,
                            b')' | b'}' => alt -= 1,
                            _ => {}
                        }
                        if alt > 0 {
                            i += 1;
                        }
                    }
                    if alt != 0 {
                        mori("parenthesis non clausa in expansione");
                    }

                    let contentum = &plicam[ab..i];
                    i += 1; /* transi ')' vel '}' */

                    /* quaere ':' pro substitutione suffixorum
                     * (extra $() nidificatas) */
                    let colon_pos = {
                        let mut pos = None;
                        let mut d: i32 = 0;
                        let cb = contentum.as_bytes();
                        let mut j = 0;
                        while j < cb.len() {
                            if cb[j] == b'$'
                                && j + 1 < cb.len()
                                && (cb[j + 1] == b'(' || cb[j + 1] == b'{')
                            {
                                d += 1;
                                j += 2;
                                continue;
                            }
                            if d > 0 && (cb[j] == b')' || cb[j] == b'}') {
                                d -= 1;
                                j += 1;
                                continue;
                            }
                            if d == 0 && cb[j] == b':' {
                                pos = Some(j);
                                break;
                            }
                            j += 1;
                        }
                        pos
                    };

                    if let Some(cp) = colon_pos {
                        /* $(VAR:vetus=novum) */
                        let nom = &contentum[..cp];
                        let spec = &contentum[cp + 1..];
                        if let Some(aeq) = spec.find('=') {
                            let vetus = &spec[..aeq];
                            let novum = &spec[aeq + 1..];
                            if let Some(d) = self.quaere_dictum(nom) {
                                let val = self.expande(&d.pretium.clone(), ctx, prof + 1);
                                let sub = substitue_suffixum(&val, vetus, novum);
                                res.push_str(&sub);
                            }
                        } else {
                            mori("'=' deest in substitutione");
                        }
                    } else {
                        /* $(VAR) simplex */
                        if let Some(d) = self.quaere_dictum(contentum) {
                            let pretium = d.pretium.clone();
                            let val = self.expande(&pretium, ctx, prof + 1);
                            res.push_str(&val);
                        }
                        /* si non inventum, expanditur in vacuum */
                    }
                }
                _ => {
                    /* $X — variabilis unius litterae */
                    let nom = &plicam[i..i + 1];
                    if let Some(d) = self.quaere_dictum(nom) {
                        let pretium = d.pretium.clone();
                        let val = self.expande(&pretium, ctx, prof + 1);
                        res.push_str(&val);
                    }
                    i += 1;
                }
            }
        }

        if res.len() >= LIM_FILUM {
            mori("expansio nimis longa");
        }
        res
    }
}

/* ============================================================
 * VI. FORMAE (EXEMPLARIA CUM %)
 * ============================================================ */

/* congrue exemplar cum scopo; reddit stirpem si congruit, aliter None */
fn congrue(scopus: &str, exemplar: &str) -> Option<String> {
    if let Some(pct) = exemplar.find('%') {
        let praefixum = &exemplar[..pct];
        let suffixum = &exemplar[pct + 1..];

        if scopus.len() < praefixum.len() + suffixum.len() {
            return None;
        }
        if !scopus.starts_with(praefixum) {
            return None;
        }
        if !scopus.ends_with(suffixum) {
            return None;
        }
        let stirps = &scopus[praefixum.len()..scopus.len() - suffixum.len()];
        Some(stirps.to_string())
    } else {
        if scopus == exemplar {
            Some(String::new())
        } else {
            None
        }
    }
}

/* substitue '%' cum stirpe in filo */
fn applica_stirpem(plicam: &str, stirps: &str) -> String {
    plicam.replace('%', stirps)
}

/* ============================================================
 * VII. TEMPUS FASCICULI
 * ============================================================ */

fn tempus_fasciculi(via: &str) -> u64 {
    match fs::metadata(via) {
        Ok(m) => m.mtime() as u64,
        Err(_) => 0,
    }
}

/* ============================================================
 * VIII. FICTA (.PHONY)
 * ============================================================ */

impl Status {
    fn est_fictum(&self, scopus: &str) -> bool {
        self.ficta.iter().any(|f| f == scopus)
    }

    fn adde_fictum(&mut self, scopus: &str) {
        if !self.est_fictum(scopus) {
            self.ficta.push(scopus.to_string());
        }
    }
}

/* ============================================================
 * IX. RESOLUTIO TABULAE (PARSING)
 * ============================================================ */

/* tolle commentum: '#' extra $() truncat lineam */
fn tolle_commentum(linea: &str) -> String {
    let bytes = linea.as_bytes();
    let mut alt: i32 = 0;
    for (i, &b) in bytes.iter().enumerate() {
        if b == b'$' && i + 1 < bytes.len() && (bytes[i + 1] == b'(' || bytes[i + 1] == b'{') {
            alt += 1;
            continue;
        }
        if alt > 0 && (b == b')' || b == b'}') {
            alt -= 1;
            continue;
        }
        if alt == 0 && b == b'#' {
            return linea[..i].to_string();
        }
    }
    linea.to_string()
}

impl Status {
    fn lege_tabulam(&mut self, via: &str) {
        let contentum = match fs::read_to_string(via) {
            Ok(c) => c,
            Err(e) => {
                eprintln!("face: '{}' aperire non possum: {}", via, e);
                process::exit(2);
            }
        };

        /* lege versus physicos, coniunge continuationes (\) */
        let mut versus: Vec<String> = Vec::new();
        let mut acervus = String::new();

        for linea in contentum.lines() {
            if linea.ends_with('\\') {
                /* continuatio */
                acervus.push_str(&linea[..linea.len() - 1]);
                continue;
            }
            acervus.push_str(linea);
            versus.push(acervus.clone());
            acervus.clear();
        }
        if !acervus.is_empty() {
            versus.push(acervus);
        }

        /* tracta versus logicos */
        let mut reg_idx: Option<usize> = None; /* index regulae currentis */
        let mut for_idx: Option<usize> = None; /* index formae currentis */

        for v in &versus {
            /* praeceptum (incipit cum tabula)? */
            if v.starts_with('\t') {
                let prc = &v[1..];
                if let Some(ri) = reg_idx {
                    if self.regulae[ri].praecepta.len() >= LIM_PRAECEPTA {
                        mori("nimis multa praecepta");
                    }
                    self.regulae[ri].praecepta.push(prc.to_string());
                } else if let Some(fi) = for_idx {
                    if self.formae[fi].praecepta.len() >= LIM_PRAECEPTA {
                        mori("nimis multa praecepta");
                    }
                    self.formae[fi].praecepta.push(prc.to_string());
                }
                /* si nullum contextum habemus, linea ignoratur */
                continue;
            }

            /* non praeceptum — frange contextum */
            reg_idx = None;
            for_idx = None;

            /* tolle commentum et tonde */
            let sine_comm = tolle_commentum(v);
            let t = tonde(&sine_comm);
            if t.is_empty() {
                continue;
            }

            /* determina typum: assignatio (=) vel regula (:) */
            let bytes = t.as_bytes();
            let mut alt: i32 = 0;
            let mut typus = 0u8; /* 0=ignotum, 1='=', 2=regula, 3='?=', 4='+=' */
            let mut sep_pos: Option<usize> = None;
            let mut lon_op = 0usize;
            let mut j = 0;

            while j < bytes.len() {
                let b = bytes[j];
                if b == b'$'
                    && j + 1 < bytes.len()
                    && (bytes[j + 1] == b'(' || bytes[j + 1] == b'{')
                {
                    alt += 1;
                    j += 2;
                    continue;
                }
                if alt > 0 && (b == b')' || b == b'}') {
                    alt -= 1;
                    j += 1;
                    continue;
                }
                if alt == 0 {
                    if b == b'?' && j + 1 < bytes.len() && bytes[j + 1] == b'=' {
                        typus = 3;
                        sep_pos = Some(j);
                        lon_op = 2;
                        break;
                    }
                    if b == b'+' && j + 1 < bytes.len() && bytes[j + 1] == b'=' {
                        typus = 4;
                        sep_pos = Some(j);
                        lon_op = 2;
                        break;
                    }
                    if b == b':' && j + 1 < bytes.len() && bytes[j + 1] == b'=' {
                        typus = 1;
                        sep_pos = Some(j);
                        lon_op = 2;
                        break;
                    }
                    if b == b'=' {
                        typus = 1;
                        sep_pos = Some(j);
                        lon_op = 1;
                        break;
                    }
                    if b == b':' {
                        typus = 2;
                        sep_pos = Some(j);
                        lon_op = 1;
                        break;
                    }
                }
                j += 1;
            }

            let sp = match sep_pos {
                Some(p) => p,
                None => continue, /* linea ignota ignoratur */
            };

            if typus == 1 || typus == 3 || typus == 4 {
                /* assignatio variabilis */
                let nomen = tonde(&t[..sp]).to_string();
                let pretium = tonde(&t[sp + lon_op..]).to_string();

                if typus == 3 {
                    self.pone_si_vacuum(&nomen, &pretium);
                } else if typus == 4 {
                    let pretium_vetus = self.quaere_dictum(&nomen).map(|d| d.pretium.clone());
                    if let Some(pv) = pretium_vetus {
                        let coniunctum = format!("{} {}", pv, pretium);
                        self.pone_dictum(&nomen, &coniunctum, false);
                    } else {
                        self.pone_dictum(&nomen, &pretium, false);
                    }
                } else {
                    self.pone_dictum(&nomen, &pretium, false);
                }
            } else if typus == 2 {
                /* regula aedificationis */
                let scopus_crudus = tonde(&t[..sp]).to_string();
                let pendentia = tonde(&t[sp + 1..]).to_string();

                /* expande scopum (pro $(NOMEN) etc.) */
                let scopus_exp = self.expande(&scopus_crudus, None, 0);
                let scop = tonde(&scopus_exp).to_string();

                /* .PHONY */
                if scop == ".PHONY" {
                    let ficta_exp = self.expande(&pendentia, None, 0);
                    let verba = scinde(&ficta_exp);
                    for verbum in verba {
                        self.adde_fictum(&verbum);
                    }
                    continue;
                }

                /* forma exemplaris (%) vel regula explicita? */
                if scop.contains('%') {
                    let idx = self.formae.len();
                    self.formae.push(Forma {
                        exemplar: scop,
                        pendentia_cruda: pendentia,
                        praecepta: Vec::new(),
                    });
                    for_idx = Some(idx);
                } else {
                    /* quaere regulam iam existentem pro hoc scopo */
                    let existens_idx = self.regulae.iter().position(|r| r.scopus == scop);

                    if let Some(ei) = existens_idx {
                        /* coniunge pendentia */
                        if !pendentia.is_empty() {
                            let con = format!("{} {}", self.regulae[ei].pendentia_cruda, pendentia);
                            self.regulae[ei].pendentia_cruda = con;
                        }
                        reg_idx = Some(ei);
                    } else {
                        let idx = self.regulae.len();
                        /* primum scopum explicitum serva */
                        if self.scopus_primus.is_none() {
                            self.scopus_primus = Some(scop.clone());
                        }
                        self.regulae.push(Regula {
                            scopus: scop,
                            pendentia_cruda: pendentia,
                            praecepta: Vec::new(),
                        });
                        reg_idx = Some(idx);
                    }
                }
            }
        }
    }
}

/* ============================================================
 * X. AEDIFICATIO
 * ============================================================ */

impl Status {
    fn quaere_regulam(&self, scopus: &str) -> Option<usize> {
        self.regulae.iter().position(|r| r.scopus == scopus)
    }

    /* quaere formam optimam (stirpem brevissimam praeferens) */
    fn quaere_formam(&self, scopus: &str) -> Option<(usize, String)> {
        let mut optima: Option<(usize, String)> = None;

        for (i, forma) in self.formae.iter().enumerate() {
            if let Some(stirps) = congrue(scopus, &forma.exemplar) {
                let melior = match &optima {
                    Some((_, s)) => stirps.len() < s.len(),
                    None => true,
                };
                if melior {
                    optima = Some((i, stirps));
                }
            }
        }
        optima
    }

    fn iam_aedificatum(&self, scopus: &str) -> bool {
        self.iam.iter().any(|s| s == scopus)
    }

    fn nota_aedificatum(&mut self, scopus: &str) {
        self.iam.push(scopus.to_string());
    }

    /* exsequere praeceptum unum per /bin/sh */
    fn exsequere(&self, praeceptum: &str) {
        let mut p = praeceptum;
        let mut tacite = false;
        let mut permitte = false;

        loop {
            if p.starts_with('@') {
                tacite = true;
                p = &p[1..];
            } else if p.starts_with('-') {
                permitte = true;
                p = &p[1..];
            } else {
                break;
            }
        }

        if !tacite && !self.modus_tacitus {
            println!("{}", p);
        }

        let status = Command::new("/bin/sh").arg("-c").arg(p).status();

        match status {
            Err(e) => {
                eprintln!("face: exsequi non possum: {}", e);
                if !permitte {
                    process::exit(2);
                }
            }
            Ok(s) => {
                if !s.success() {
                    if !permitte {
                        match s.code() {
                            Some(c) => eprintln!("face: *** Error {}", c),
                            None => eprintln!("face: *** Signo interfectum"),
                        }
                        process::exit(2);
                    }
                }
            }
        }
    }

    fn aedifica(&mut self, scopus: &str) {
        /* si iam aedificatum, nihil agendum */
        if self.iam_aedificatum(scopus) {
            return;
        }

        /* detege circulos */
        if self.semita.iter().any(|s| s == scopus) {
            eprintln!("face: circulus detectus: '{}'", scopus);
            process::exit(2);
        }
        self.semita.push(scopus.to_string());

        /* quaere regulam explicitam */
        let regula_idx = self.quaere_regulam(scopus);
        let forma_res = self.quaere_formam(scopus);

        let mut pend_cruda: String;
        let mut praecepta: Vec<String>;

        if let Some(ri) = regula_idx {
            pend_cruda = self.regulae[ri].pendentia_cruda.clone();
            praecepta = self.regulae[ri].praecepta.clone();

            /* si nulla praecepta, quaere formam */
            if praecepta.is_empty() {
                if let Some((fi, ref stirps)) = forma_res {
                    let pend_st = applica_stirpem(&self.formae[fi].pendentia_cruda, stirps);
                    /* coniunge pendentia regulae et formae */
                    pend_cruda = format!("{} {}", pend_cruda, pend_st);
                    praecepta = self.formae[fi].praecepta.clone();
                }
            }
        } else if let Some((fi, ref stirps)) = forma_res {
            pend_cruda = applica_stirpem(&self.formae[fi].pendentia_cruda, stirps);
            praecepta = self.formae[fi].praecepta.clone();
        } else {
            /* nulla regula — si fasciculus existit, nihil agendum */
            if tempus_fasciculi(scopus) == 0 {
                eprintln!("face: regulam pro '{}' invenire non possum", scopus);
                process::exit(2);
            }
            self.semita.pop();
            self.nota_aedificatum(scopus);
            return;
        }

        /* expande pendentia */
        let pend_exp = self.expande(&pend_cruda, None, 0);
        let verba = scinde(&pend_exp);
        let num_pend = verba.len();

        /* aedifica omnia pendentia */
        let mut tempus_max: u64 = 0;

        if self.numerus_operariorum > 1 && num_pend > 1 {
            /* modus parallelus: aedifica pendentia per fila (threads) */
            /* Nota Bernoulli: in Rust, fila loco fork() utimur,
             * sed Status non est inter fila communicabilis simpliciter.
             * Ergo serialiter aedificamus — fidelis functionalitas. */
            for verbum in &verba {
                self.aedifica(verbum);
                let tp = tempus_fasciculi(verbum);
                if tp > tempus_max {
                    tempus_max = tp;
                }
            }
        } else {
            /* modus serialis */
            for verbum in &verba {
                self.aedifica(verbum);
                let tp = tempus_fasciculi(verbum);
                if tp > tempus_max {
                    tempus_max = tp;
                }
            }
        }

        /* an debeamus praecepta exsequi? */
        let fictum = self.est_fictum(scopus);
        let tempus_scopi = tempus_fasciculi(scopus);
        let debet = fictum || tempus_scopi == 0 || tempus_max > tempus_scopi;

        if debet && !praecepta.is_empty() {
            /* para variabilia automatica */
            let primum = if num_pend > 0 {
                verba[0].clone()
            } else {
                String::new()
            };
            let omnia = verba.join(" ");

            let ctx = Contextus {
                scopus: scopus.to_string(),
                primum,
                omnia,
            };

            for praeceptum in &praecepta {
                let exp = self.expande(praeceptum, Some(&ctx), 0);
                self.exsequere(&exp);
            }
        }

        /* purga semitam */
        self.semita.pop();
        self.nota_aedificatum(scopus);
    }
}

/* ============================================================
 * XI. PRINCEPS
 * ============================================================ */

fn main() {
    let argumenta: Vec<String> = env::args().collect();
    let mut via: Option<String> = None;
    let mut directorium: Option<String> = None;
    let mut scopi: Vec<String> = Vec::new();

    let mut status = Status::novus();

    let mut i = 1;
    while i < argumenta.len() {
        let arg = &argumenta[i];
        if arg == "-C" {
            i += 1;
            if i >= argumenta.len() {
                mori("-C: nomen directorii deest");
            }
            directorium = Some(argumenta[i].clone());
        } else if arg == "-f" {
            i += 1;
            if i >= argumenta.len() {
                mori("-f: nomen tabulae deest");
            }
            via = Some(argumenta[i].clone());
        } else if arg == "-j" {
            i += 1;
            if i >= argumenta.len() {
                mori("-j: numerus deest");
            }
            let n: usize = argumenta[i]
                .parse()
                .unwrap_or_else(|_| mori("-j: numerus positivus esse debet"));
            if n < 1 {
                mori("-j: numerus positivus esse debet");
            }
            status.numerus_operariorum = n;
        } else if arg.starts_with("-j") && arg.len() > 2 {
            /* -jN forma */
            let n: usize = arg[2..]
                .parse()
                .unwrap_or_else(|_| mori("-j: numerus positivus esse debet"));
            if n < 1 {
                mori("-j: numerus positivus esse debet");
            }
            status.numerus_operariorum = n;
        } else if arg == "-s" || arg == "--silent" {
            status.modus_tacitus = true;
        } else if let Some(aeq) = arg.find('=') {
            /* VAR=pretium in linea imperata */
            let nomen = &arg[..aeq];
            let pretium = &arg[aeq + 1..];
            status.pone_dictum(nomen, pretium, true);
        } else {
            scopi.push(arg.clone());
        }
        i += 1;
    }

    /* muta directorium si -C datum est */
    if let Some(ref dir) = directorium {
        if env::set_current_dir(dir).is_err() {
            eprintln!("face: in '{}' intrare non possum", dir);
            process::exit(2);
        }
    }

    /* quaere tabulam */
    let via_def = if let Some(v) = via {
        v
    } else if fs::metadata("Faceplica").is_ok() {
        "Faceplica".to_string()
    } else if fs::metadata("faceplica").is_ok() {
        "faceplica".to_string()
    } else {
        mori("Faceplica invenire non possum");
    };

    status.lege_tabulam(&via_def);

    /* pone FACE variabilem (viam ad se ipsum) */
    status.pone_dictum("FACE", &argumenta[0], true);

    /* pone CURDIR variabilem */
    if let Ok(cdir) = env::current_dir() {
        if let Some(s) = cdir.to_str() {
            status.pone_dictum("CURDIR", s, true);
        }
    }

    if scopi.is_empty() {
        match &status.scopus_primus {
            Some(s) => scopi.push(s.clone()),
            None => mori("nullum scopum invenire possum"),
        }
    }

    for scopus in &scopi {
        status.aedifica(scopus);
    }
}
