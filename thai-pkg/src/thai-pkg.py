#!/usr/bin/env python3
"""
ThaiPkg — ThaiOS Package Manager
Manages software packages for ThaiOS
"""

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
import tarfile
import tempfile
from datetime import datetime

REPO_DIR = "/var/lib/thai-pkg"
DB_FILE = os.path.join(REPO_DIR, "packages.db")
REMOTE_REPO = "https://repo.thaios.dev/packages"
THAIOS_ROOT = ""
VERSION = "1.0"


class ThaiPackage:
    def __init__(self, name, version, arch="x86_64", deps=None, size=0, checksum="", desc=""):
        self.name = name
        self.version = version
        self.arch = arch
        self.deps = deps or []
        self.size = size
        self.checksum = checksum
        self.desc = desc
        self.installed = False
        self.install_date = ""

    def to_dict(self):
        return {
            "name": self.name,
            "version": self.version,
            "arch": self.arch,
            "deps": self.deps,
            "size": self.size,
            "checksum": self.checksum,
            "desc": self.desc,
            "installed": self.installed,
            "install_date": self.install_date,
        }

    @staticmethod
    def from_dict(data):
        pkg = ThaiPackage(data["name"], data["version"], data["arch"],
                          data["deps"], data["size"], data["checksum"], data["desc"])
        pkg.installed = data.get("installed", False)
        pkg.install_date = data.get("install_date", "")
        return pkg


class ThaiPkg:
    def __init__(self, root=""):
        self.root = root
        self.db_path = os.path.join(root + DB_FILE)
        self.repo_dir = os.path.join(root + REPO_DIR)
        self.packages = {}
        self.ensure_db()

    def ensure_db(self):
        os.makedirs(self.repo_dir, exist_ok=True)
        if os.path.exists(self.db_path):
            self.load_db()
        else:
            self.init_default_packages()
            self.save_db()

    def load_db(self):
        try:
            with open(self.db_path) as f:
                data = json.load(f)
            for name, pkg_data in data.items():
                self.packages[name] = ThaiPackage.from_dict(pkg_data)
        except (json.JSONDecodeError, FileNotFoundError):
            self.init_default_packages()

    def save_db(self):
        data = {name: pkg.to_dict() for name, pkg in self.packages.items()}
        with open(self.db_path, "w") as f:
            json.dump(data, f, indent=2)

    def init_default_packages(self):
        defaults = [
            ("ThaiOS-base", "1.0", ["ThaiOS-kernel"], 0, "", "Sistema base ThaiOS"),
            ("ThaiOS-kernel", "6.6.0", [], 0, "", "Kernel ThaiOS"),
            ("ThaiDesktop", "1.0", ["ThaiOS-base"], 0, "", "Desktop environment ThaiOS"),
            ("ThaiBrowser", "1.0", ["ThaiDesktop"], 0, "", "Browser web ThaiOS"),
            ("ThaiFiles", "1.0", ["ThaiDesktop"], 0, "", "File manager ThaiOS"),
            ("ThaiStore", "1.0", ["ThaiDesktop"], 0, "", "App store ThaiOS"),
            ("ThaiTerminal", "1.0", ["ThaiDesktop"], 0, "", "Terminale ThaiOS"),
            ("ThaiSettings", "1.0", ["ThaiDesktop"], 0, "", "Impostazioni ThaiOS"),
            ("ThaiMedia", "1.0", ["ThaiDesktop"], 0, "", "Media player ThaiOS"),
            ("ThaiEditor", "1.0", ["ThaiDesktop"], 0, "", "Editor di testo ThaiOS"),
            ("ThaiBackup", "1.0", ["ThaiDesktop"], 0, "", "Backup ThaiOS"),
        ]
        for name, ver, deps, size, csum, desc in defaults:
            pkg = ThaiPackage(name, ver, deps=deps, size=size, checksum=csum, desc=desc)
            pkg.installed = True
            pkg.install_date = datetime.now().isoformat()
            self.packages[name] = pkg

    def install(self, pkg_name):
        if pkg_name in self.packages and self.packages[pkg_name].installed:
            print(f"[ThaiPkg] {pkg_name} \u00e8 gi\u00e0 installato")
            return True

        # Resolve dependencies
        to_install = self.resolve_deps(pkg_name)
        if to_install is None:
            print(f"[ThaiPkg] Errore: dipendenze non risolte per {pkg_name}")
            return False

        for dep in to_install:
            if dep not in self.packages:
                pkg = ThaiPackage(dep, "1.0", desc=f"Pacchetto {dep}")
                self.packages[dep] = pkg

            self.packages[dep].installed = True
            self.packages[dep].install_date = datetime.now().isoformat()
            print(f"[ThaiPkg] Installato {dep} v{self.packages[dep].version}")

        self.save_db()
        print(f"[ThaiPkg] Installazione completata: {len(to_install)} pacchetti")
        return True

    def remove(self, pkg_name):
        if pkg_name not in self.packages or not self.packages[pkg_name].installed:
            print(f"[ThaiPkg] {pkg_name} non \u00e8 installato")
            return False

        dependents = self.find_dependents(pkg_name)
        if dependents:
            print(f"[ThaiPkg] Attenzione: i seguenti pacchetti dipendono da {pkg_name}:")
            for d in dependents:
                print(f"  - {d}")
            confirm = input("Continuare? [y/N]: ")
            if confirm.lower() != "y":
                return False

        self.packages[pkg_name].installed = False
        self.packages[pkg_name].install_date = ""
        self.save_db()
        print(f"[ThaiPkg] Rimosso {pkg_name}")
        return True

    def update(self):
        print(f"[ThaiPkg] Aggiornamento repository...")
        # Simulate remote repo sync
        for name, pkg in self.packages.items():
            print(f"  {name}: {pkg.version} -> {pkg.version} (attuale)")
        print(f"[ThaiPkg] Sistema aggiornato ({len(self.packages)} pacchetti)")
        return True

    def upgrade(self):
        print(f"[ThaiPkg] Aggiornamento pacchetti...")
        updated = 0
        for name, pkg in self.packages.items():
            if pkg.installed:
                print(f"  Aggiornato {name} a v{pkg.version}")
                updated += 1
        print(f"[ThaiPkg] Aggiornati {updated} pacchetti")
        return True

    def search(self, query):
        results = []
        for name, pkg in self.packages.items():
            if query.lower() in name.lower() or query.lower() in pkg.desc.lower():
                results.append(pkg)
        return results

    def list_packages(self, only_installed=False):
        if only_installed:
            return [p for p in self.packages.values() if p.installed]
        return list(self.packages.values())

    def info(self, pkg_name):
        if pkg_name in self.packages:
            return self.packages[pkg_name]
        return None

    def resolve_deps(self, pkg_name, resolved=None, seen=None):
        if resolved is None:
            resolved = []
        if seen is None:
            seen = set()

        if pkg_name in seen:
            return resolved
        seen.add(pkg_name)

        if pkg_name in self.packages:
            for dep in self.packages[pkg_name].deps:
                if dep not in self.packages or not self.packages[dep].installed:
                    if dep not in resolved:
                        resolved.append(dep)
                    self.resolve_deps(dep, resolved, seen)

        if pkg_name not in resolved:
            resolved.append(pkg_name)

        return resolved

    def find_dependents(self, pkg_name):
        dependents = []
        for name, pkg in self.packages.items():
            if pkg.installed and pkg_name in pkg.deps:
                dependents.append(name)
        return dependents


def main():
    parser = argparse.ArgumentParser(description="ThaiPkg - ThaiOS Package Manager")
    parser.add_argument("action", nargs="?", choices=["install", "remove", "update", "upgrade",
                                                       "search", "list", "info", "autoremove"],
                        help="Azione da eseguire")
    parser.add_argument("packages", nargs="*", help="Nomi dei pacchetti")
    parser.add_argument("--root", default="", help="Root directory alternative")

    args = parser.parse_args()

    if not args.action:
        parser.print_help()
        return

    pm = ThaiPkg(args.root)

    if args.action == "install":
        if not args.packages:
            print("[ThaiPkg] Specificare uno o pi\u00f9 pacchetti da installare")
            return
        for pkg in args.packages:
            pm.install(pkg)

    elif args.action == "remove":
        if not args.packages:
            print("[ThaiPkg] Specificare uno o pi\u00f9 pacchetti da rimuovere")
            return
        for pkg in args.packages:
            pm.remove(pkg)

    elif args.action == "update":
        pm.update()

    elif args.action == "upgrade":
        pm.upgrade()

    elif args.action == "search":
        if not args.packages:
            print("[ThaiPkg] Specificare un termine di ricerca")
            return
        query = " ".join(args.packages)
        results = pm.search(query)
        if results:
            print(f"[ThaiPkg] Risultati per '{query}':")
            for pkg in results:
                status = "[I]" if pkg.installed else "[ ]"
                print(f"  {status} {pkg.name} v{pkg.version} - {pkg.desc}")
        else:
            print(f"[ThaiPkg] Nessun risultato per '{query}'")

    elif args.action == "list":
        only_installed = "--installed" in sys.argv
        packages = pm.list_packages(only_installed)
        print(f"[ThaiPkg] Pacchetti: {len(packages)}")
        for pkg in sorted(packages, key=lambda p: p.name):
            status = "[I]" if pkg.installed else "[ ]"
            print(f"  {status} {pkg.name} v{pkg.version}")

    elif args.action == "info":
        if not args.packages:
            print("[ThaiPkg] Specificare un pacchetto")
            return
        pkg = pm.info(args.packages[0])
        if pkg:
            print(f"Nome: {pkg.name}")
            print(f"Versione: {pkg.version}")
            print(f"Architettura: {pkg.arch}")
            print(f"Descrizione: {pkg.desc}")
            print(f"Dipendenze: {', '.join(pkg.deps) if pkg.deps else 'nessuna'}")
            print(f"Installato: {'S\u00ec' if pkg.installed else 'No'}")
            if pkg.install_date:
                print(f"Data installazione: {pkg.install_date}")
        else:
            print(f"[ThaiPkg] Pacchetto '{args.packages[0]}' non trovato")


if __name__ == "__main__":
    main()
