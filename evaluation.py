import numpy as np
import pandas as pd
import os
from collections import defaultdict
import torch


PATH_TO_DATASET = './PianoFingeringDataset_v1.2/PianoFingeringDataset_v1.2'
PATH_TO_DATASET_FOLDER = './PianoFingeringDataset_v1.2/PianoFingeringDataset_v1.2/FingeringFiles'
PATH_TO_METADATA = './PianoFingeringDataset_v1.2/PianoFingeringDataset_v1.2/List.csv'
FINGERING_TYPE_TO_ANALYZE = "1"


class FingeringEvaluator:
    def __init__(self, dataset_path=PATH_TO_DATASET):
        self.dataset_path = dataset_path
        self.fingering_files_path = PATH_TO_DATASET_FOLDER
        self.metadata_path = PATH_TO_METADATA

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
        
        for filename in os.listdir(self.fingering_files_path):
            if not filename.endswith('_fingering.txt'):
                continue
                
            fingering_label, _ = filename.split('_')
            piece_id, annotator_id = fingering_label.split('-')
            
            if pieces and piece_id not in pieces:
                continue
            if annotator_ids and annotator_id not in annotator_ids:
                continue
                
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
            
            fingering = df['finger_number'].tolist()
            ground_truth_fingerings.append(fingering)
            piece_ids.append((piece_id, annotator_id))
            lengths.append(len(fingering))
        
        converted_ground_truth = []
        for sequence in ground_truth_fingerings:
            converted_sequence = [self.convert_fingering_to_int(str(finger)) for finger in sequence]
            converted_ground_truth.append(converted_sequence)
        ground_truth_fingerings = converted_ground_truth

        return ground_truth_fingerings, piece_ids, lengths


def evaluate_fingering_method(predicted_fingerings, ground_truth_fingerings, piece_ids, 
                              lengths=None, hand='right', method_name="Model", 
                              dataset_path='./PianoFingeringDataset_v1.2/PianoFingeringDataset_v1.2'):
    evaluator = FingeringEvaluator(dataset_path)
    results = evaluator.evaluate(predicted_fingerings, ground_truth_fingerings, piece_ids, lengths, hand)
    evaluator.print_results(results, method_name)
    return results





def load_model(model_path):
    model = torch.load(model_path)
    model.eval()
    return model

def predict_fingerings(model, data_loader):
    predictions = []
    with torch.no_grad():
        for inputs in data_loader:
            outputs = model(inputs)
            _, predicted = torch.max(outputs, 1)
            predictions.append(predicted.cpu().numpy())
    return predictions

if __name__ == "__main__":
    # Load the model
    model_path = './rnn_model_weights.pth'
    model = load_model(model_path)

    # Load test data
    evaluator = FingeringEvaluator()
    ground_truth_fingerings, piece_ids, lengths = evaluator.load_test_data()

    # Prepare data loader
    test_data = torch.tensor(ground_truth_fingerings, dtype=torch.float32)
    test_loader = torch.utils.data.DataLoader(test_data, batch_size=32, shuffle=False)

    # Predict fingerings
    predicted_fingerings = predict_fingerings(model, test_loader)

    # Evaluate the model
    results = evaluate_fingering_method(predicted_fingerings, ground_truth_fingerings, piece_ids, lengths)
    evaluator.print_results(results, method_name="RNN Model")