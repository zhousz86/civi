# -*- coding: utf-8 -*-
# Authoring helper: edit the table below, then run `python3 catalog_src.py` to regenerate
# catalog.json (the single source both languages are served from — CIVI-HMI-001).
# Order in each tuple is (id, French, English[, French singular, English singular]): French is authored first, never as an
# afterthought translation.
import json, os

M = [
# --- navigation rail / common
("civi.nav.home", "Accueil", "Home"),
("civi.nav.navigation", "Navigation", "Navigation"),
("civi.nav.media", "Audio", "Media"),
("civi.nav.climate", "Climatisation", "Climate"),
("civi.nav.connectivity", "Connectivité", "Connectivity"),
("civi.nav.privacy", "Confidentialité", "Privacy"),
("civi.nav.updates", "Mises à jour", "Updates"),
("civi.nav.settings", "Réglages", "Settings"),
("civi.common.understood", "J’ai compris", "Understood"),
("civi.common.cancel", "Annuler", "Cancel"),
("civi.common.off", "Arrêt", "Off"),

# --- distraction lockout
("civi.lockout.available_parked", "Disponible à l’arrêt", "Available when parked"),
("civi.lockout.reason.moving", "Véhicule en mouvement", "Vehicle in motion"),
("civi.lockout.reason.unknown", "État de mouvement inconnu — fonctions limitées par sécurité", "Motion state unknown — features limited for safety"),

# --- link states and bearers
("civi.link.state.up", "Connecté", "Connected"),
("civi.link.state.degraded", "Dégradé", "Degraded"),
("civi.link.state.blocked", "Ciel masqué", "Sky blocked"),
("civi.link.state.down", "Hors service", "Down"),
("civi.link.state.off", "Non installé", "Not fitted"),
("civi.link.state.unknown", "Inconnu", "Unknown"),
("civi.bearer.geo", "Satellite GEO", "GEO satellite"),
("civi.bearer.leo", "Satellite LEO", "LEO satellite"),
("civi.bearer.terrestrial", "Cellulaire 4G/5G", "Cellular 4G/5G"),
("civi.bearer.wifi", "Wi-Fi (domicile ou concessionnaire)", "Wi-Fi (home or dealer)"),
("civi.bearer.queued", "En file d’attente", "Queued"),
("civi.bearer.none", "Aucune liaison", "No link"),
("civi.bearer.unknown", "Inconnue", "Unknown"),

# --- connectivity page
("civi.con.role.leo", "LEO · Internet", "LEO · Internet"),
("civi.con.role.meo", "MEO · Positionnement", "MEO · Positioning"),
("civi.con.role.geo", "GEO · Messagerie et urgence", "GEO · Messaging and emergency"),
("civi.con.role.terr", "Cellulaire (option)", "Cellular (optional)"),
("civi.con.leo_detail", "%1 Mb/s descendant · %2 Mb/s montant", "%1 Mbps down · %2 Mbps up"),
("civi.con.geo_detail", "Élévation de l’antenne : %1°", "Antenna elevation: %1°"),
("civi.con.fix.3d", "Position 3D · %1 satellites", "3D fix · %1 satellites"),
("civi.con.fix.dr", "Navigation à l’estime (%1 satellites)", "Dead reckoning (%1 satellites)"),
("civi.con.fix.none", "Aucune position (%1 satellites)", "No position (%1 satellites)"),
("civi.con.terr_fitted", "Délestage en zone urbaine ; jamais requis", "Urban cost offload; never required"),
("civi.con.terr_not_fitted", "Non installé sur cette version — aucune fonction n’en dépend", "Not fitted on this trim — no function depends on it"),
("civi.con.routing", "Acheminement du trafic", "Traffic routing"),
("civi.con.class.emergency", "Urgence", "Emergency"),
("civi.con.class.safety", "Messages de sécurité", "Safety messaging"),
("civi.con.class.telemetry", "Télémétrie", "Telemetry"),
("civi.con.class.ota", "Mises à jour à distance", "Over-the-air updates"),
("civi.con.class.media", "Audio et vidéo", "Media"),
("civi.con.class.passenger", "Internet des passagers", "Passenger internet"),
("civi.con.class.bulk", "Données en vrac", "Bulk data"),
("civi.con.queue", "En attente d’envoi : %1 (envoyé au retour de la liaison)", "Waiting to send: %1 (sent when the link returns)"),
("civi.con.emergency_path", "Voie d’urgence", "Emergency path"),
("civi.con.emergency_path_is", "Voie d’urgence : %1", "Emergency path: %1"),
("civi.con.offline_banner", "Hors ligne — navigation, commandes vocales, appels d’urgence et réglages restent disponibles", "Offline — navigation, voice, emergency calling and settings still work"),
("civi.con.status_unknown", "État de la connectivité inconnu — nouvelle tentative en cours", "Connectivity status unknown — retrying"),

# --- passenger Wi-Fi
("civi.wifi.title", "Wi-Fi des passagers", "Passenger Wi-Fi"),
("civi.wifi.enable", "Wi-Fi des passagers", "Passenger Wi-Fi"),
("civi.wifi.isolated", "Isolé des systèmes du véhicule ; le trafic du véhicule reste prioritaire", "Isolated from vehicle systems; vehicle traffic always has priority"),
("civi.wifi.status", "%n appareils connectés sur %1", "%n of %1 devices connected", "%n appareil connecté sur %1", "%n of %1 devices connected"),
("civi.wifi.short", "Wi-Fi · %1", "Wi-Fi · %1"),
("civi.wifi.usage", "%1 utilisés sur %2", "%1 used of %2"),
("civi.wifi.usage_nocap", "%1 utilisés · sans limite", "%1 used · no limit"),
("civi.wifi.no_cap", "Sans limite", "No limit"),
("civi.wifi.off", "Wi-Fi désactivé", "Wi-Fi off"),
("civi.wifi.passenger_detail", "Internet par satellite LEO. Le propriétaire gère l’accès et la limite de données.", "Internet over LEO satellite. The owner controls access and the data limit."),

# --- home / navigation
("civi.home.to", "Vers %1", "To %1"),
("civi.home.no_route", "Aucun itinéraire", "No route"),
("civi.nav.offline_ready", "Cartes hors ligne prêtes", "Offline maps ready"),
("civi.nav.offline_pending", "Cartes hors ligne en préparation", "Preparing offline maps"),
("civi.nav.destination", "Destination", "Destination"),
("civi.nav.search_hint", "Adresse, lieu ou commerce", "Address, place or business"),
("civi.nav.search_voice_only", "En mouvement : dites « Aller à… »", "While moving: say “Navigate to…”"),
("civi.nav.guidance", "Itinéraire", "Route"),
("civi.nav.eta", "Arrivée dans %1 · %2", "Arrive in %1 · %2"),
("civi.nav.cached_region", "En cache : %1", "Cached: %1"),
("civi.nav.sky_gaps", "Zones sans ciel prévues", "Predicted sky-view gaps"),
("civi.nav.no_gaps", "Aucune zone prévue sur l’itinéraire", "None predicted on this route"),
("civi.nav.gap.tunnel", "Tunnel", "Tunnel"),
("civi.nav.gap.terrain", "Relief", "Terrain"),
("civi.nav.gap.canopy", "Couvert forestier", "Tree canopy"),
("civi.nav.gap.parkade", "Stationnement intérieur", "Parkade"),
("civi.nav.gap.latitude", "Latitude élevée", "High latitude"),
("civi.nav.prefetch_done", "Données préchargées", "Data prefetched"),
("civi.nav.prefetch_running", "Préchargement en cours", "Prefetching"),

# --- media
("civi.media.now_playing", "En cours", "Now playing"),
("civi.media.stations", "Stations", "Stations"),
("civi.media.stream", "En ligne", "Streaming"),
("civi.media.play", "Lecture", "Play"),
("civi.media.pause", "Pause", "Pause"),
("civi.media.blend", "Mélange radio et diffusion en ligne", "Broadcast and stream blend"),

# --- climate
("civi.climate.setpoint", "Température", "Temperature"),
("civi.climate.cabin", "Habitacle", "Cabin"),
("civi.climate.seat_heat", "Siège chauffant", "Heated seat"),
("civi.climate.defrost", "Dégivrage", "Defrost"),
("civi.climate.defrost_detail", "Pare-brise et lunette arrière", "Windshield and rear window"),
("civi.climate.precondition", "Préconditionnement à distance", "Remote pre-conditioning"),
("civi.climate.precondition_detail", "Démarrage à distance, préchauffage, mises à jour et terminal satellite partagent la batterie 12 V. Le gestionnaire de réveil les limite selon la charge (%1 %) et le froid.", "Remote start, pre-heating, updates and the satellite terminal share the 12 V battery. The wake-policy engine budgets them by charge (%1%) and cold."),

# --- cluster
("civi.cluster.range", "Autonomie", "Range"),
("civi.cluster.outside", "Extérieur", "Outside"),
("civi.cluster.time", "Heure", "Time"),
("civi.av.mode.manual", "Conduite manuelle", "Manual driving"),
("civi.av.mode.l2", "Aide à la conduite active", "Driver assist active"),
("civi.av.mode.l3", "Conduite automatisée", "Automated driving"),
("civi.av.mode.takeover", "Reprise en main demandée", "Takeover requested"),

# --- takeover and warnings
("civi.takeover.title", "REPRENEZ LE VOLANT", "TAKE THE WHEEL"),
("civi.takeover.reason.odd_exit", "Fin de la zone de conduite automatisée", "Leaving the automated driving zone"),
("civi.takeover.reason.weather", "Conditions hivernales — capteurs limités", "Winter conditions — sensors limited"),
("civi.takeover.reason.sensor_degraded", "Capteur obstrué par la neige ou la glace", "Sensor blocked by snow or ice"),
("civi.takeover.reason.construction", "Zone de travaux", "Construction zone"),
("civi.takeover.reason.system_fault", "Anomalie du système", "System fault"),
("civi.warning.collision", "FREINEZ — risque de collision", "BRAKE — collision risk"),
("civi.warning.low_traction", "Chaussée glissante", "Slippery road"),
("civi.warning.lane_departure", "Sortie de voie", "Lane departure"),
("civi.warning.brake_fault", "Anomalie de freinage — arrêtez-vous prudemment", "Brake fault — stop safely"),
("civi.warning.driver_attention", "Regardez la route", "Eyes on the road"),

# --- public alerts (NPAS / Alert Ready)
("civi.alert.public.kind.wildfire", "Alerte feu de forêt", "Wildfire alert"),
("civi.alert.public.kind.tornado", "Alerte de tornade", "Tornado warning"),
("civi.alert.public.kind.amber", "Alerte AMBER", "AMBER Alert"),
("civi.alert.public.kind.flood", "Alerte d’inondation", "Flood alert"),
("civi.alert.public.kind.generic", "Alerte d’urgence", "Emergency alert"),
("civi.alert.media_paused", "Audio en pause", "Media paused"),

# --- emergency
("civi.emg.title", "Urgence", "Emergency"),
("civi.emg.title.sos", "Appel d’urgence en cours", "Emergency call in progress"),
("civi.emg.title.acn", "Collision détectée — les secours sont avisés", "Crash detected — help is being notified"),
("civi.emg.sos_hold", "Maintenez SOS pour appeler le 911", "Hold SOS to call 911"),
("civi.emg.hold_hint", "Maintenir 3 s", "Hold 3 s"),
("civi.emg.state.idle", "Prêt", "Ready"),
("civi.emg.state.connecting", "Connexion au 911…", "Connecting to 911…"),
("civi.emg.state.active", "En communication avec le 911", "Connected to 911"),
("civi.emg.state.queued", "En attente d’une liaison satellite", "Waiting for a satellite link"),
("civi.emg.state.ended", "Appel terminé", "Call ended"),
("civi.emg.state.failed", "Échec — nouvelle tentative automatique", "Failed — retrying automatically"),
("civi.emg.no_bearer", "Aucune liaison : déplacez-vous vers un endroit à ciel ouvert. L’appel partira dès qu’un satellite sera visible.", "No link: move to open sky. The call will go out as soon as a satellite is visible."),
("civi.emg.fallback_order", "Ordre de secours : GEO, puis LEO, puis cellulaire si installé.", "Fallback order: GEO, then LEO, then cellular where fitted."),
("civi.emg.location", "Position", "Location"),
("civi.emg.location_sent", "Position et données de collision transmises", "Location and crash data sent"),
("civi.emg.ng911_detail", "Appel IP prêt pour le 911 de prochaine génération : voix, texte, position et données de collision.", "IP-native, Next Generation 911-ready: voice, text, location and crash data."),

# --- privacy
("civi.privacy.consent", "Consentements", "Consents"),
("civi.profile.driver1", "Conducteur 1", "Driver 1"),
("civi.profile.driver2", "Conducteur 2", "Driver 2"),
("civi.profile.guest", "Invité", "Guest"),
("civi.purpose.nav_history", "Historique de navigation", "Navigation history"),
("civi.purpose.voice_recordings", "Enregistrements vocaux", "Voice recordings"),
("civi.purpose.driving_behaviour", "Comportement de conduite", "Driving behaviour"),
("civi.purpose.location_sharing", "Partage de position", "Location sharing"),
("civi.purpose.av_data_loop", "Données pour la conduite automatisée", "Automated-driving data"),
("civi.privacy.retention", "Conservé %n jours · %1", "Kept %n days · %1", "Conservé %n jour · %1", "Kept %n day · %1"),
("civi.residency.CA", "Hébergé au Canada", "Hosted in Canada"),
("civi.residency.US", "Hébergé aux États-Unis", "Hosted in the United States"),
("civi.privacy.residency", "Résidence des données", "Data residency"),
("civi.privacy.residency_detail", "Télémétrie, voix et compte sont acheminés vers une infrastructure canadienne. Les clés de chiffrement restent au Canada.", "Telemetry, voice and account data are routed to Canadian infrastructure. Encryption keys stay in Canada."),
("civi.privacy.policy", "Politique %1", "Policy %1"),
("civi.privacy.always_on", "Toujours actif", "Always on"),
("civi.privacy.emergency_location", "Votre position est transmise aux services d’urgence lors d’un appel 911 ou d’une collision. Cette fonction ne peut pas être désactivée.", "Your location is sent to emergency services during a 911 call or a crash. This cannot be turned off."),
("civi.privacy.reset", "Réinitialiser pour le prochain propriétaire", "Reset for next owner"),
("civi.privacy.reset_detail", "Efface toutes les données personnelles et tous les profils.", "Erases all personal data and driver profiles."),
("civi.privacy.reset_confirm", "Toutes les données personnelles seront effacées. Confirmer ?", "All personal data will be erased. Confirm?"),
("civi.privacy.reset_now", "Effacer maintenant", "Erase now"),

# --- OTA
("civi.ota.title", "Mise à jour du logiciel", "Software update"),
("civi.ota.checks", "Vérifications", "Checks"),
("civi.ota.state.idle", "Logiciel à jour", "Software up to date"),
("civi.ota.state.available", "Mise à jour disponible", "Update available"),
("civi.ota.state.downloading", "Téléchargement…", "Downloading…"),
("civi.ota.state.ready", "Prête à installer", "Ready to install"),
("civi.ota.state.installing", "Installation…", "Installing…"),
("civi.ota.state.done", "Installée", "Installed"),
("civi.ota.state.failed", "Échec — version précédente restaurée", "Failed — previous version restored"),
("civi.ota.state.rollback", "Retour à la version précédente", "Rolled back"),
("civi.ota.version", "Version %1 · %2", "Version %1 · %2"),
("civi.ota.notes", "Notes de version", "Release notes"),
("civi.ota.install", "Installer", "Install"),
("civi.ota.postpone", "Plus tard", "Later"),
("civi.ota.gate.signature", "Signature vérifiée", "Signature verified"),
("civi.ota.gate.sbom", "Provenance et nomenclature logicielle", "Provenance and software bill of materials"),
("civi.ota.gate.parity", "Parité française vérifiée", "French parity verified"),
("civi.ota.gate.battery", "Charge de batterie suffisante", "Battery charge sufficient"),
("civi.ota.gate.temperature", "Température dans la plage permise", "Temperature within range"),
("civi.ota.gate.parked", "Véhicule stationné (P)", "Vehicle parked (P)"),
("civi.ota.block.signature", "Signature non vérifiée", "Signature not verified"),
("civi.ota.block.sbom", "Provenance non vérifiée", "Provenance not verified"),
("civi.ota.block.parity", "Parité française non vérifiée", "French parity not verified"),
("civi.ota.block.battery", "Charge de batterie insuffisante", "Battery charge too low"),
("civi.ota.block.temperature", "Température hors plage", "Temperature out of range"),
("civi.ota.block.parked", "Placez le véhicule en position P", "Shift to P"),

# --- settings / accessibility
("civi.settings.language", "Langue", "Language"),
("civi.settings.language_detail", "Une seule langue pour tout l’habitacle : écrans, voix (%1), alertes et notifications.", "One language for the whole cockpit: displays, voice (%1), alerts and notifications."),
("civi.settings.units", "Unités", "Units"),
("civi.settings.metric", "Métriques", "Metric"),
("civi.settings.imperial", "Impériales", "Imperial"),
("civi.settings.about", "À propos", "About"),
("civi.settings.lockout_threshold", "Verrouillage en conduite au-delà de %1", "Driving lockout above %1"),
("civi.settings.takeover_budget", "Délai cible de reprise en main : %1 ms", "Takeover display budget: %1 ms"),
("civi.settings.takeover_last", "Dernier délai mesuré : %1 ms", "Last measured: %1 ms"),
("civi.settings.accessibility", "Accessibilité", "Accessibility"),
("civi.a11y.large_text", "Grand texte", "Large text"),
("civi.a11y.high_contrast", "Contraste élevé", "High contrast"),
("civi.a11y.screen_reader", "Lecteur d’écran", "Screen reader"),
("civi.a11y.haptics", "Confirmation haptique", "Haptic confirmation"),
("civi.a11y.voice_rate", "Vitesse de la voix : %1×", "Voice speed: %1×"),

# --- passenger
("civi.passenger.title", "Écran passager", "Passenger display"),
("civi.passenger.video_ready", "Vidéo disponible", "Video ready"),
("civi.passenger.video_offline", "Hors ligne — contenus téléchargés seulement", "Offline — downloaded content only"),
]

out = {"_comment": "GENERATED by catalog_src.py — single source for French and English (CIVI-HMI-001). Do not edit by hand.",
       "languages": ["fr", "en"], "messages": {}}
for row in M:
    mid, fr, en = row[:3]
    assert mid not in out["messages"], "duplicate id " + mid
    e = {"fr": fr, "en": en}
    if len(row) == 5:            # singular forms for qsTrId(id, n)
        e["fr_one"], e["en_one"] = row[3], row[4]
    out["messages"][mid] = e

here = os.path.dirname(os.path.abspath(__file__))
with open(os.path.join(here, "catalog.json"), "w", encoding="utf-8") as f:
    json.dump(out, f, ensure_ascii=False, indent=1, sort_keys=True)
    f.write("\n")
print(len(M), "messages written")
