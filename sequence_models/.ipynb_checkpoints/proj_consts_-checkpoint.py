import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader

import pandas as pd
import numpy as np
import os
import matplotlib.pyplot as plt
import seaborn as sns

from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
from sklearn.preprocessing import StandardScaler
from collections import defaultdict

class FingeringEvaluator:
    def __init__(self, dataset_path, train_dataset_path, metadata_path):
        self.dataset_path = dataset_path
        self.fingering_files_path = train_dataset_path
        self.metadata_path = metadata_path

        self.fingerings = [
            '1', '2', '3', '4', '5',
            '1_', '2_', '3_', '4_', '5_',
            '-1', '-2', '-3', '-4', '-5',
            '1_2', '1_3', '1_4', '1_5',
            '1_-2', '1_-3', '1_-4', '1_-5',
            '2_1', '2_3', '2_4', '2_5',
            '2_-1', '2_-3', '2_-4', '2_-5',
            '3_1', '3_2', '3_4', '3_5',
            '3_-1', '3_-2', '3_-4', '3_-5',
            '4_1', '4_2', '4_3', '4_5',
            '4_-1', '4_-2', '4_-3', '4_-5',
            '5_1', '5_2', '5_3', '5_4',
            '5_-1', '5_-2', '5_-3', '5_-4',
            '-1_-2', '-1_-3', '-1_-4', '-1_-5',
            '-1_2', '-1_3', '-1_4', '-1_5',
            '-2_-3', '-2_-4', '-2_-5', '-2_-1',
            '-2_3', '-2_4', '-2_5', '-2_1',
            '-3_-4', '-3_-5', '-3_-2', '-3_-1',
            '-3_4', '-3_5', '-3_2', '-3_1',
            '-4_-5', '-4_-3', '-4_-2', '-4_-1',
            '-4_5', '-4_3', '-4_2', '-4_1',
            '-5_-1', '-5_-2', '-5_-3', '-5_-4',
            '-5_1', '-5_2', '-5_3', '-5_4',
            '1_2_3', '1_2_4', '1_2_5', '1_3_4', '1_3_5', '1_4_5', '2_3_4', '2_3_5', '2_4_5', '3_4_5',
            '-1_1', '-2_2', '-3_3', '-4_4', '-5_5', '1_-1', '2_-2', '3_-3', '4_-4', '5_-5', '0'
        ]
        self.finger_to_int_mapping = {f: i for i, f in enumerate(self.fingerings)}
        self.int_to_finger_mapping = {i: f for i, f in enumerate(self.fingerings)}

        if os.path.exists(self.metadata_path):
            self.metadata = pd.read_csv(
                self.metadata_path, 
                skiprows=1, 
                names=["id", "composer", "piece", "num_bars", "num_notes", 
                      "num_types_of_fingerings_provided", "fingering_1", "fingering_2", 
                      "fingering_3", "fingering_4", "fingering_5", "fingering_6", 
                      "fingering_7", "fingering_8"]
            )
        
        self.load_note2ids()


    def load_note2ids(self):
        self.note2ids = {}
        for filename in os.listdir(self.fingering_files_path):
            if not filename.endswith('_fingering.txt'):
                continue
                
            fingering_label, _ = filename.split('_')
            piece_id, fingering_type = fingering_label.split('-')
            
            file_path = os.path.join(self.fingering_files_path, filename)
            if not os.path.isfile(file_path):
                continue
                
            df = pd.read_table(
                file_path, 
                sep="\t", 
                skiprows=1, 
                names=["noteID", "onset_time", "offset_time", "spelled_pitch", 
                      "onset_velocity", "offset_velocity", "channel", "finger_number"]
            )
            
            id_key = f"{piece_id}-{fingering_type}"
            if id_key not in self.note2ids:
                self.note2ids[id_key] = {}
                
            for _, row in df.iterrows():
                note_id = int(row['noteID'])
                self.note2ids[id_key][note_id] = {
                    'pitch': row['spelled_pitch'],
                    'channel': row['channel'],
                    'finger': row['finger_number']
                }
    
    
    def average_annotator_per_piece(self, ids, match_rates):
        numbers, _ = zip(*ids)
        unique_numbers = list(set(numbers))
        mmr_all = []
        for n in unique_numbers:
            mrs = [mr for id_piece, mr in zip(unique_numbers, match_rates) if id_piece == n]
            mmr_all.append(sum(mrs) / len(mrs))
        return mmr_all, unique_numbers


    def general_match_rate(self, y_pred, y_true, ids, lengths=None):
        # indicating how closely the estimation agrees with all the ground truths
        # compute match rate for every piece fingered
        if lengths is None:
            lengths = [len(yy) for yy in y_pred]
        match_rates = []
        for p, t, l, id_piece in zip(y_pred, y_true, lengths, ids):
            assert len(p) == len(t) == l, f"id {id_piece}: apples with lemons gmr: {len(p)} != {len(t)} != {l}"
            matches = 0
            for idx, (pp, tt) in enumerate(zip(p, t)):
                if idx >= l:
                    break
                else:
                    if pp == tt:
                        matches += 1
            match_rates.append(matches / l)
        return self.average_annotator_per_piece(ids, match_rates)

    def avg_general_match_rate(self, y_pred, y_true, ids, lengths=None):
        gmr, _ = self.general_match_rate(y_pred, y_true, ids, lengths=lengths)
        return sum(gmr) / len(gmr)

    
    def highest_match_rate(self, y_pred, y_true, ids, lengths=None):
        if lengths is None:
            lengths = [len(yy) for yy in y_pred]
        
        piece_to_matches = defaultdict(list)
        
        for p, t, l, (piece_id, _) in zip(y_pred, y_true, lengths, ids):
            matches = 0
            for idx, (pp, tt) in enumerate(zip(p, t)):
                if idx >= l:
                    break
                elif pp == tt:
                    matches += 1
            piece_to_matches[piece_id].append(matches / l)
        
        highest_rates = [max(rates) if rates else 0 for rates in piece_to_matches.values()]
        return sum(highest_rates) / len(highest_rates) if highest_rates else 0
    
    
    def soft_match_rate(self, y_pred, y_true, ids, lengths=None, hand='right'):
        if lengths is None:
            lengths = [len(yy) for yy in y_pred]
        
        piece_to_gt = defaultdict(list)
        for (piece_id, annotator_id), t in zip(ids, y_true):
            piece_to_gt[piece_id].append(t)
        
        soft_match_rates = []
        for (piece_id, _), p, l in zip(ids, y_pred, lengths):
            ground_truths = piece_to_gt[piece_id]
            soft_matches = 0
            for idx in range(l):
                if any(gt[idx] == p[idx] for gt in ground_truths if idx < len(gt)):
                    soft_matches += 1
            
            soft_match_rates.append(soft_matches / l)
        
        piece_ids = [pid for pid, _ in ids]
        unique_pieces = set(piece_ids)
        
        avg_rates = []
        for piece in unique_pieces:
            piece_rates = [rate for (pid, _), rate in zip(ids, soft_match_rates) if pid == piece]
            if piece_rates:
                avg_rates.append(sum(piece_rates) / len(piece_rates))
        
        return sum(avg_rates) / len(avg_rates) if avg_rates else 0
    
    
    def evaluate(self, y_pred, y_true, ids, lengths=None, hand='right'):
        if lengths is None:
            lengths = [len(yy) for yy in y_pred]
        
        M_gen = self.avg_general_match_rate(y_pred, y_true, ids, lengths)
        M_high = self.highest_match_rate(y_pred, y_true, ids, lengths)
        M_soft = self.soft_match_rate(y_pred, y_true, ids, lengths, hand)
        
        results = {
            'M_gen': M_gen,
            'M_high': M_high,
            'M_soft': M_soft,
        }
        
        return results
    
    
    def print_results(self, results, method_name="Model"):
        print(f"\n{'=' * 40}")
        print(f"{method_name} Evaluation Results")
        print(f"{'=' * 40}")
        print(f"General Match Rate (M_gen):      {results['M_gen']:.4f}")
        print(f"Highest Match Rate (M_high):     {results['M_high']:.4f}")
        print(f"Soft Match Rate (M_soft):        {results['M_soft']:.4f}")
        print(f"{'=' * 40}\n")


    def convert_fingering_to_int(self, finger_str):
        if finger_str in self.finger_to_int_mapping:
            return self.finger_to_int_mapping[finger_str]
        else:
            return self.finger_to_int_mapping.get('0', 0)


    def load_test_data(self, pieces=None, annotator_ids=None):
        ground_truth_fingerings = []
        piece_ids = []
        lengths = []
        print(self.dataset_path)
        
        for filename in os.listdir(self.dataset_path):
            # print(filename)
            # if not filename.endswith('_fingering.txt'):
            #     continue
                
            fingering_label, _ = filename.split('_')
            piece_id, annotator_id = fingering_label.split('-')
            
            # if pieces and piece_id not in pieces:
            #     continue
            # if annotator_ids and annotator_id not in annotator_ids:
            #     continue
                
            file_path = os.path.join(self.dataset_path, filename)
            if not os.path.isfile(file_path):
                print("NOT A VALID FILEPATH: ", file_path)
                continue
                
            df = pd.read_table(
                file_path, 
                sep="\t", 
                skiprows=1, 
                names=["noteID", "onset_time", "offset_time", "spelled_pitch", 
                      "onset_velocity", "offset_velocity", "channel", "finger_number"]
            )
            # print(df)
            
            fingering = df['finger_number'].tolist()
            ground_truth_fingerings.append(fingering)
            piece_ids.append((piece_id, annotator_id))
            lengths.append(len(fingering))
        
        converted_ground_truth = []
        for sequence in ground_truth_fingerings:
            converted_sequence = [self.convert_fingering_to_int(str(finger)) for finger in sequence]
            converted_ground_truth.append(converted_sequence)
        ground_truth_fingerings = converted_ground_truth

        print(ground_truth_fingerings, piece_ids, lengths)

        return ground_truth_fingerings, piece_ids, lengths


def evaluate_fingering_method(predicted_fingerings, ground_truth_fingerings, piece_ids, 
                              lengths=None, hand='right', method_name="Model", 
                              evaluator=None):
    if not evaluator:
        print("no evaluator defined!")
        return
    results = evaluator.evaluate(predicted_fingerings, ground_truth_fingerings, piece_ids, lengths, hand)
    evaluator.print_results(results, method_name)
    return results


# every pitch is a combination of a note and octave
pitch_classes = ['Ab', 'A', 'A#', 'Bb', 'B', 'B#', 'Cb', 'C', 'C#', 'Db', 'D', 'D#', 'Eb', 'E', 'E#', 'Fb', 'F', 'F#', 'Gb', 'G', 'G#']
octaves = range(0, 9)
pitch_to_int_mapping = {f"{pc}{octave}": i for i, (pc, octave) in enumerate((pc, o) for o in octaves for pc in pitch_classes)}
int_to_pitch_mapping = {i: f"{pc}{octave}" for i, (pc, octave) in enumerate((pc, o) for o in octaves for pc in pitch_classes)}

fingerings = [
    '1', '2', '3', '4', '5',
    '1_', '2_', '3_', '4_', '5_', 
    '-1', '-2', '-3', '-4', '-5', 
    '1_2', '1_3', '1_4', '1_5', 
    '1_-2', '1_-3', '1_-4', '1_-5', 
    '2_1', '2_3', '2_4', '2_5', 
    '2_-1', '2_-3', '2_-4', '2_-5', 
    '3_1', '3_2', '3_4', '3_5', 
    '3_-1', '3_-2', '3_-4', '3_-5', 
    '4_1', '4_2', '4_3', '4_5', 
    '4_-1', '4_-2', '4_-3', '4_-5', 
    '5_1', '5_2', '5_3', '5_4',
    '5_-1', '5_-2', '5_-3', '5_-4',
    '-1_-2', '-1_-3', '-1_-4', '-1_-5',
    '-1_2', '-1_3', '-1_4', '-1_5', 
    '-2_-3', '-2_-4', '-2_-5', '-2_-1',
    '-2_3', '-2_4', '-2_5', '-2_1',
    '-3_-4', '-3_-5', '-3_-2', '-3_-1',
    '-3_4', '-3_5', '-3_2', '-3_1',
    '-4_-5', '-4_-3', '-4_-2', '-4_-1',
    '-4_5', '-4_3', '-4_2', '-4_1',
    '-5_-1', '-5_-2', '-5_-3', '-5_-4',
    '-5_1', '-5_2', '-5_3', '-5_4',
    '1_2_3', '1_2_4', '1_2_5', '1_3_4', '1_3_5', '1_4_5', '2_3_4', '2_3_5', '2_4_5', '3_4_5',
    '-1_1', '-2_2', '-3_3', '-4_4', '-5_5', '1_-1', '2_-2', '3_-3', '4_-4', '5_-5', '0'
]
finger_to_int_mapping = {f: i for i, f in enumerate(fingerings)}
int_to_finger_mapping = {i: f for i, f in enumerate(fingerings)}



song_metadata_dir_path = './PianoFingeringDataset_v1.2/List.csv'
if os.path.isfile(song_metadata_dir_path):
    song_metadata_df = pd.read_csv(song_metadata_dir_path, skiprows=1, names=["id", "composer", "piece", "num_bars", "num_notes", "num_types_of_fingerings_provided", "fingering_1", "fingering_2", "fingering_3", "fingering_4", "fingering_5", "fingering_6", "fingering_7", "fingering_8"])
    # print(song_metadata_df.head(10))
else:
    print("invalid metadata filepath!")


# Creating the dictionary mapping initials to a list of associated piece filenames
annotator_to_files_dict = {}

# Iterate over the rows to construct the mapping
for _, row in song_metadata_df.iterrows():
    piece_id = f"{int(row['id']):03d}"  # Ensuring three-digit format for IDs
    for i in range(1, 9):  # fingering_1 to fingering_8
        fingering_col = f"fingering_{i}"
        annotator = row[fingering_col]
        if pd.notna(annotator):  # Ensure it's not NaN
            filename = f"{piece_id}-{i}"
            if annotator not in annotator_to_files_dict:
                annotator_to_files_dict[annotator] = []
            annotator_to_files_dict[annotator].append(filename)



def predict_piece_fingerings_multi(
    model,
    file_path,
    pitch_to_idx,         # dict mapping spelled pitches -> int
    finger_to_idx=None,   # dict mapping finger labels -> int (optional)
    idx_to_finger=None,   # reverse mapping to decode predictions (optional)
    device=None
):
    df = pd.read_table(file_path, sep="\t", skiprows=1,
        names=[
            "noteID",
            "onset_time",
            "offset_time",
            "spelled_pitch",
            "onset_velocity",
            "offset_velocity",
            "channel",
            "finger_number",
        ],
    )

    # Encode spelled pitch -> int and finger labels -> int
    df["spelled_pitch_int"] = df["spelled_pitch"].map(pitch_to_int_mapping).fillna(0).astype(int)

    if finger_to_idx is not None:
        df["finger_int"] = df["finger_number"].astype(str).map(finger_to_int_mapping).fillna(0).astype(int)
        y_true = df["finger_int"].values
    else:
        y_true = None

    #    Construct multi-feature array X of shape (seq_len, 6)
    #    X[..., 0] = spelled_pitch_int (categorical)
    #    X[..., 1] = onset_time
    #    X[..., 2] = offset_time
    #    X[..., 3] = onset_velocity
    #    X[..., 4] = offset_velocity
    #    X[..., 5] = channel
    seq_len = len(df)
    X_array = np.zeros((seq_len, 6), dtype=np.float32)

    X_array[:, 0] = df["spelled_pitch_int"].astype(float)
    X_array[:, 1] = df["onset_time"].astype(float)
    X_array[:, 2] = df["offset_time"].astype(float)
    X_array[:, 3] = df["onset_velocity"].astype(float)
    X_array[:, 4] = df["offset_velocity"].astype(float)
    X_array[:, 5] = df["channel"].astype(float)

    X_tensor = torch.tensor(X_array, dtype=torch.float32).unsqueeze(0)    # convert X_array into a pytorch tensor

    # move everything to gpu if possible
    if device is None:
        device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    X_tensor = X_tensor.to(device)
    model = model.to(device)
    model.eval()

    with torch.no_grad():
        logits = model(X_tensor)
        preds = torch.argmax(logits, dim=-1).squeeze(0).cpu().numpy()

    if idx_to_finger:
        y_pred_labels = [int_to_finger_mapping.get(idx, "UNK") for idx in preds]
    else:
        y_pred_labels = preds
    acc = None
    y_true_labels = None
    if y_true is not None:
        from sklearn.metrics import accuracy_score
        acc = accuracy_score(y_true, preds) * 100.0
        if idx_to_finger:
            y_true_labels = [int_to_finger_mapping.get(idx, "UNK") for idx in y_true]

    return preds, y_true, y_pred_labels, y_true_labels, acc







if __name__ == "__main__":

    directory_path = './PianoFingeringDataset_v1.2/FingeringFiles'
    
    # verify that every finger and pitch in our data can be mapped correctly
    verify_spelled_pitch_values = set()
    verify_fingering_map = set()
    for filename in os.listdir(directory_path):
        file_path = os.path.join(directory_path, filename)
        if os.path.isfile(file_path):
            df = pd.read_table(file_path, sep="\t", skiprows=1, names=["noteID", "onset_time", "offset_time", "spelled_pitch", "onset_velocity", "offset_velocity", "channel", "finger_number"])
            verify_spelled_pitch_values.update(df['spelled_pitch'].unique())
            verify_fingering_map.update(df['finger_number'].unique())
    verify_fingering_map = {str(x) for x in verify_fingering_map}
    
    if not verify_fingering_map.issubset(set(finger_to_int_mapping.keys())):
        print("INVALID FINGER SYMBOL DETECTED: ", verify_fingering_map - set(finger_to_int_mapping.keys()))
    elif not verify_spelled_pitch_values.issubset(set(pitch_to_int_mapping.keys())):
        print("INVALID PITCH SYMBOL DETECTED: ", verify_spelled_pitch_values - set(pitch_to_int_mapping.keys()))
    else:
        print("pitch_to_int_mapping: ", pitch_to_int_mapping)
        print("\n")
        print("finger_to_int_mapping: ", finger_to_int_mapping)

