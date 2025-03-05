from enum import Enum, auto
from dataclasses import dataclass
from typing import List, Dict, Tuple, Optional
import numpy as np

class Hand(Enum):
    LEFT = -1
    RIGHT = 1

class Finger(Enum):
    THUMB = 1  # f1
    INDEX = 2  # f2
    MIDDLE = 3 # f3 
    RING = 4   # f4
    PINKY = 5  # f5

@dataclass
class Note:
    pitch: int
    position: int
    duration: int = 0
    velocity: int = 0
    channel: int = 0

@dataclass 
class NoteFingerPair:
    note: Note
    finger: Finger

class PianoFingeringModel:
    def __init__(self):
        # Maximum stretching distances between finger pairs
        # -1 means physically impossible
        self.max_finger_distance = np.array([
            [-1, 4, 5, 6, 7],
            [3, -1, 3, 4, 6],
            [2, -1, -1, 3, 4],
            [1.5, -1, -1, -1, 3],
            [-1, -1, -1, -1, -1]
        ])
        
        # Mapping of MIDI note numbers to physical key distances
        self.white_keys = list(range(21, 109, 2))
        self.black_keys = list(range(22, 107, 2))

    # CALCULATE PHYSICAL DISTANCE BETWEEN TWO KEYS IN TERMS OF WHITE KEY SPACING
    def get_key_distance(self, note1: Note, note2: Note) -> float:
        def get_position(note):
            if note.pitch in self.white_keys:
                return self.white_keys.index(note.pitch)
            return self.white_keys.index(note.pitch + 1) - 0.5
            
        return abs(get_position(note1) - get_position(note2))

    # CALCULATE FINGER STRETCHING RATE BETWEEN 0-1 BASED ON PHYSICAL CONSTRAINTS
    def get_stretching_rate(self, nfp1: NoteFingerPair, nfp2: NoteFingerPair, hand: Hand) -> float:
  
        # Adjust order based on hand
        if ((nfp1.note.pitch > nfp2.note.pitch and hand == Hand.RIGHT) or 
            (nfp1.note.pitch < nfp2.note.pitch and hand == Hand.LEFT)):
            nfp1, nfp2 = nfp2, nfp1

        f1, f2 = int(nfp1.finger.value - 1), int(nfp2.finger.value - 1)
        natural_distance = abs(nfp1.finger.value - nfp2.finger.value)
        physical_distance = self.get_key_distance(nfp1.note, nfp2.note)
        max_distance = self.max_finger_distance[f1, f2]

        if physical_distance > natural_distance:
            return (physical_distance - natural_distance) / (max_distance - natural_distance)
        return -(natural_distance - physical_distance) / natural_distance
        
    # DETERMINE IF MOVEMENT REQUIRES CROSS-FINGERING TECHNIQUES
    def is_cross_fingering(self, nfp1: NoteFingerPair, nfp2: NoteFingerPair, hand: Hand) -> bool:
        direction = 1 if nfp2.note.pitch > nfp1.note.pitch else -1
        
        # Check crossing over thumb
        if nfp1.finger in [Finger.INDEX, Finger.MIDDLE, Finger.RING] and nfp2.finger == Finger.THUMB:
            return direction == 1 and hand == Hand.RIGHT or direction == -1 and hand == Hand.LEFT
            
        # Check thumb crossing under
        if nfp1.finger == Finger.THUMB:
            return direction == 1 and hand == Hand.LEFT or direction == -1 and hand == Hand.RIGHT
            
        return False

    # CALCULATES PHYSICAL DISTANCE REQUIRED FOR CROSS-FINGERING MOVEMENT
    def get_cross_distance(self, nfp1: NoteFingerPair, nfp2: NoteFingerPair) -> float:        
        distance = self.get_key_distance(nfp1.note, nfp2.note)
        if nfp1.finger == Finger.THUMB:
            return distance + nfp2.finger.value - 1
        return distance + nfp1.finger.value - 1

    # CALCULATES REWARD TRANSITIONING BETWEEN TWO STATES
    def calculate_reward(self, current_fingering: List[NoteFingerPair], 
                        next_fingering: List[NoteFingerPair], 
                        hand: Hand) -> float:
  
        # Single note to single note transition
        if len(current_fingering) == 1 and len(next_fingering) == 1:
            nfp1, nfp2 = current_fingering[0], next_fingering[0]
            
            # Check for cross-fingering
            if self.is_cross_fingering(nfp1, nfp2, hand):
                cross_distance = self.get_cross_distance(nfp1, nfp2)
                return 20 + 2.5 * (4 - cross_distance)
            
            # Calculate stretching reward
            stretch_rate = self.get_stretching_rate(nfp1, nfp2, hand)
            if stretch_rate == 0:
                return 50
            elif stretch_rate > 0:
                return 40 + 10 * (1 - stretch_rate**2)
            
            # Penalize hand movement
            hand_move = self.get_key_distance(nfp1.note, nfp2.note)
            return -10 - 0.5 * hand_move

        # Multiple note transitions; calculate avg stretching and movement penalties
        total_stretch = 0
        for nfp1 in current_fingering:
            for nfp2 in next_fingering:
                stretch_rate = abs(self.get_stretching_rate(nfp1, nfp2, hand))
                total_stretch += stretch_rate ** 1.5
                
        avg_stretch = total_stretch / (len(current_fingering) * len(next_fingering))
        hand_movement = self.calculate_hand_movement(current_fingering, next_fingering)
        
        # n-to-1 transition
        if len(next_fingering) == 1:
            return (50 - hand_movement)
            
        # 1-to-n or n-to-n transition
        if hand_movement > 5:
            return 20 * (1 - avg_stretch) + (45 - hand_movement) / 4.5
        return 25 * (6 * (1 - avg_stretch**2.2) + 4 * (5 - hand_movement)) / 13

    # CALCULATES HOW FAR HAND POSITION NEEDS TO MOVE
    def calculate_hand_movement(self, current_fingering: List[NoteFingerPair],
                              next_fingering: List[NoteFingerPair]) -> float:  
        def get_hand_position(fingering):
            if not fingering:
                return 0
            # Approximate hand position as average of thumb and pinky positions
            positions = [self.get_key_distance(Note(21, 0), nfp.note) + 
                       (3 - nfp.finger.value) for nfp in fingering]
            return sum(positions) / len(positions)
            
        return abs(get_hand_position(current_fingering) - get_hand_position(next_fingering))